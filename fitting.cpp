#include <iostream>

#include "fitting.h"

Fitting::Fitting(Raman &raman, Diamond &diamond, Laser &laser)
    : m_num_frequencies(raman.get_num_sample_points()), 
      m_num_pressures(diamond.get_num_elements()) {

    m_fitting_params = gsl_multifit_nlinear_default_parameters();

    m_simulation_info.raman = &raman;
    m_simulation_info.diamond = &diamond;
    m_simulation_info.laser = &laser;

    m_starting_pressures = new double[m_num_pressures]();
    m_data_weights = new double[m_num_frequencies + m_num_constraints];

    for (int i = 0; i != m_num_frequencies; i++) {
        m_data_weights[i] = 1.0;
    }
    // Make weight of final penalty terms equal to all others combined
    m_data_weights[m_num_frequencies] = m_num_frequencies;
    m_data_weights[m_num_frequencies + 1] = m_num_frequencies;

    m_pressures = gsl_vector_view_array(m_starting_pressures, m_num_pressures);
    m_weights = gsl_vector_view_array(m_data_weights, m_num_frequencies + m_num_constraints);

    // Define function to be minimised
    m_fitting_equations.f = compute_cost_function;
    m_fitting_equations.df = NULL;    // Compute Jacobian from finite difference
    m_fitting_equations.fvv = NULL;   // Do not use geodesic acceleration
    m_fitting_equations.n = m_num_frequencies + m_num_constraints;
    m_fitting_equations.p = m_num_pressures;
    m_fitting_equations.params = &m_simulation_info;

    m_covariance = gsl_matrix_alloc(m_num_pressures, m_num_pressures);
}

Fitting::~Fitting() {
    // Free memory
    gsl_multifit_nlinear_free(m_workspace);
    gsl_matrix_free(m_covariance);
    if (m_starting_pressures) {
        delete [] m_starting_pressures;
    }
    if (m_data_weights) {
        delete [] m_data_weights;
    }
}

void Fitting::set_initial_pressures(const std::vector<double> &init_pressures) {
    for (int i = 0; i != m_num_pressures; i++) {
        m_starting_pressures[i] = init_pressures[i];
    }
}

void Fitting::initialize() {
    set_initial_pressures(m_simulation_info.diamond->get_pressure_profile());
    // Allocate the workspace with default parameters
    m_workspace = gsl_multifit_nlinear_alloc(m_fittingtype,
                                             &m_fitting_params,
                                             m_num_frequencies + m_num_constraints,
                                             m_num_pressures);

    // initialize solver with starting point and weights
    gsl_multifit_nlinear_winit(&m_pressures.vector, &m_weights.vector, &m_fitting_equations, m_workspace);

    // compute initial cost function
    m_residuals = gsl_multifit_nlinear_residual(m_workspace);
    gsl_blas_ddot(m_residuals, m_residuals, &m_chisq0);
}

void Fitting::print_summary() const {
    // Print summary of fitting
#define FIT(i) gsl_vector_get(m_workspace->x, i)
#define ERR(i) sqrt(gsl_matrix_get(m_covariance,i,i))

    std::cout << "summary from method '" << gsl_multifit_nlinear_name(m_workspace)
              << "/" << gsl_multifit_nlinear_trs_name(m_workspace) << "'\n";
    std::cout << "number of iterations: " << gsl_multifit_nlinear_niter(m_workspace) << "\n";
    std::cout << "function evaluations: " << m_fitting_equations.nevalf << "\n";
    std::cout << "Jacobian evaluations: " << m_fitting_equations.nevaldf << "\n";
    std::cout << "reason for stopping: " << ((m_info == 1) ? "small step size" : "small gradient") << "\n";
    std::cout << "initial |f(x)| = " << sqrt(m_chisq0) << "\n";
    std::cout << "final   |f(x)| = " << sqrt(m_chisq) << std::endl;

    std::cout << "Pressures\n";
    for (int i = 0; i != m_num_pressures; i++) {
        std::cout << "Initial: " << m_starting_pressures[i] 
                  << "\tCurrent: " << FIT(i) << "\n";
    }
    std::cout << std::endl;

    std::vector<double> new_pressure_profile(m_num_pressures);
    for (int i = 0; i != m_num_pressures; i++) {
        new_pressure_profile[i] = gsl_vector_get(m_workspace->x, i);
    }
}

void Fitting::fit(const int max_iter) {
    // solve the system with a maximum of max_iter iterations
    m_status = gsl_multifit_nlinear_driver(max_iter, m_xtol, m_gtol, m_ftol, callback, NULL, &m_info, m_workspace);

    // compute covariance of best fit parameters
    m_jacobian = gsl_multifit_nlinear_jac(m_workspace);
    gsl_multifit_nlinear_covar(m_jacobian, 0.0, m_covariance);

    // compute final cost
    gsl_blas_ddot(m_residuals, m_residuals, &m_chisq);

}

int Fitting::compute_cost_function(const gsl_vector *pressures, void *data,
                                   gsl_vector *output_differences) {
    // Cast pointer to void to pointer to struct and extract the member variables
    Raman raman = *((struct SimulationInfo *)data)->raman;
    Diamond diamond = *((struct SimulationInfo *)data)->diamond;
    Laser laser = *((struct SimulationInfo *)data)->laser;
    double negative_penalty = 0;
    double decrease_penalty = 0;

    std::vector<double> pressure_profile(pressures->size);
    for (int i = 0; i != pressures->size; i++) {
        pressure_profile[i] = gsl_vector_get(pressures, i);
    }

    diamond.set_pressure_profile(pressure_profile);
    raman.compute_raman_signal(diamond, laser);

    std::vector<double> actual = raman.get_data_intensities();
    std::vector<double> predicted = raman.get_raman_signal();
    for (int i = 0; i != raman.get_num_sample_points(); i++) {
        gsl_vector_set(output_differences, i, predicted[i] - actual[i]);        
    }

    // Compute additional penalties
    for (int i = 0; i != diamond.get_num_elements(); i++) {
        if (gsl_vector_get(pressures, i) < 0) {
            negative_penalty += pow(0.0 - gsl_vector_get(pressures, i), 6);
        }
        if (i > 0) {
            double difference = gsl_vector_get(pressures, i) - gsl_vector_get(pressures, i - 1);
            decrease_penalty += difference < 0.0 ? pow(difference, 2) : 0.0;
        }
    }

    // Additional penalty for having pressures decrease towards the tip
    gsl_vector_set(output_differences, raman.get_num_sample_points(), negative_penalty);

    // Add additional penalty for frequencies below zero
    gsl_vector_set(output_differences, raman.get_num_sample_points() + 1, decrease_penalty);

    return GSL_SUCCESS;    
}

void Fitting::callback(const size_t iter, void *pressures, 
              const gsl_multifit_nlinear_workspace *workspace) {
    // gsl_vector *residual = gsl_multifit_nlinear_residual(workspace);
    // gsl_vector *current_pressures = gsl_multifit_nlinear_position(workspace);
    // std::cout << "Iteration " << iter << "\n";
}