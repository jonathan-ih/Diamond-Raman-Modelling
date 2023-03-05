#include <iostream>
#include <fstream>
#include <iomanip>

#include "fitting.h"

Fitting::Fitting(const Settings &settings, Raman &raman, Diamond &diamond, Laser &laser)
    : m_num_frequencies(raman.get_num_sample_points()), 
      m_num_pressures(diamond.get_num_elements()),
      m_verbosity(settings.general.verbosity),
      m_max_iter(settings.fitting.max_iter),
      m_print_freq(settings.fitting.print_freq),
      m_pressure_log(settings.fitting.pressure_log_file),
      m_signal_log(settings.fitting.signal_log_file),
      m_xtol(settings.fitting.xtol),
      m_gtol(settings.fitting.gtol) {

    m_fitting_params = gsl_multifit_nlinear_default_parameters();

    m_simulation_info.raman = &raman;
    m_simulation_info.diamond = &diamond;
    m_simulation_info.laser = &laser;
    m_callback_params.verbosity = m_verbosity;
    m_callback_params.max_iter = m_max_iter;
    m_callback_params.print_freq = m_print_freq;
    m_callback_params.num_freqs = m_num_frequencies;
    m_callback_params.num_pressures = m_num_pressures;
    if (!m_pressure_log.empty()) {
        m_callback_params.pressure_log.open(m_pressure_log);
        m_callback_params.pressure_log << "# ITER | PRESS" << std::endl;
    }
    if (!m_signal_log.empty()) {
        m_callback_params.signal_log.open(m_signal_log);
        m_callback_params.signal_log << "# ITER | SIGNAL" << std::endl;
    }
    m_callback_params.sim_info = &m_simulation_info;

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
    if (m_callback_params.pressure_log.is_open()) {
        m_callback_params.pressure_log.close();
    }
    if (m_callback_params.signal_log.is_open()) {
        m_callback_params.signal_log.close();
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
    gsl_vector resid_no_penalties = gsl_vector_subvector(m_residuals, 0, m_num_frequencies).vector;
    gsl_blas_ddot(&resid_no_penalties, &resid_no_penalties, &m_chisq0);
}

void Fitting::fit() {
    print_fitting_header ();
    // solve the system with a maximum of max_iter iterations
    m_status = gsl_multifit_nlinear_driver(m_max_iter, m_xtol, m_gtol, m_ftol, callback, &m_callback_params, &m_info, m_workspace);

    // compute covariance of best fit parameters
    m_jacobian = gsl_multifit_nlinear_jac(m_workspace);
    gsl_multifit_nlinear_covar(m_jacobian, 0.0, m_covariance);

    // compute final cost
    gsl_vector resid_no_penalties = gsl_vector_subvector(m_residuals, 0, m_num_frequencies).vector;
    gsl_blas_ddot(&resid_no_penalties, &resid_no_penalties, &m_chisq);
    std::cout << "Fitting complete!\n" <<std::endl;
}

void Fitting::print_fitting_header() const {
    std::cout << "Starting fit" << std::endl;
    if (m_verbosity == 1) {
        std::cout << " Iteration         chi-squared" << std::endl;
    } else if (m_verbosity == 2) {
        std::cout << " Iteration         chi-squared  max pressure  min pressure" << std::endl;
    } else if (m_verbosity == 3) {
        std::cout << " Iteration         chi-squared  max pressure  min pressure  negative penalty  Non-monotonic penalty" << std::endl;
    }
}


void Fitting::print_summary() const {
    // Print summary of fitting
#define FIT(i) gsl_vector_get(m_workspace->x, i)
#define ERR(i) sqrt(gsl_matrix_get(m_covariance,i,i))

    // Find reason for stopping
    std::string reason;
    if (m_status == GSL_EMAXITER) {
        reason = "max iterations";
    } else if (m_status == GSL_ENOPROG) {
        reason = "no progress";
    } else if (m_status == GSL_SUCCESS) {
        reason = (m_info == 1) ? "small step size (xtol reached)" : "small gradient (gtol reached)";
    } else {
        reason = "unknown problem";
    }

    std::cout << "Summary from method '" << gsl_multifit_nlinear_name(m_workspace)
              << "/" << gsl_multifit_nlinear_trs_name(m_workspace) << "'\n";
    std::cout << "number of iterations: " << gsl_multifit_nlinear_niter(m_workspace) << "\n";
    std::cout << "function evaluations: " << m_fitting_equations.nevalf << "\n";
    std::cout << "Jacobian evaluations: " << m_fitting_equations.nevaldf << "\n";
    std::cout << "reason for stopping: " << reason << "\n";
    std::cout << "initial chi-squared = " << sqrt(m_chisq0) << "\n";
    std::cout << "final   chi-squared = " << sqrt(m_chisq) << "\n" << std::endl;

    if (m_verbosity == 3) {
        std::cout << "Pressures\n";
        for (int i = 0; i != m_num_pressures; i++) {
            std::cout << "    Initial: " << std::setw(12) << m_starting_pressures[i] 
                    << "  Current: " << std::setw(12) << FIT(i) << "\n";
        }
        std::cout << std::endl;
    }
}

int Fitting::compute_cost_function(const gsl_vector *pressures, void *data,
                                   gsl_vector *output_differences) {
    // Cast pointer to void to pointer to struct and extract the member variables
    Raman *raman = ((struct SimulationInfo *)data)->raman;
    Diamond *diamond = ((struct SimulationInfo *)data)->diamond;
    Laser *laser = ((struct SimulationInfo *)data)->laser;
    double negative_penalty = 0;
    double decrease_penalty = 0;

    std::vector<double> pressure_profile(pressures->size);
    for (int i = 0; i != pressures->size; i++) {
        pressure_profile[i] = gsl_vector_get(pressures, i);
    }

    diamond->set_pressure_profile(pressure_profile);
    raman->compute_raman_signal(*diamond, *laser);

    std::vector<double> actual = raman->get_data_intensities();
    std::vector<double> predicted = raman->get_raman_signal();
    for (int i = 0; i != raman->get_num_sample_points(); i++) {
        gsl_vector_set(output_differences, i, predicted[i] - actual[i]);        
    }

    // Compute additional penalties
    for (int i = 0; i != diamond->get_num_elements(); i++) {
        if (gsl_vector_get(pressures, i) < 0) {
            negative_penalty += pow(0.0 - gsl_vector_get(pressures, i), 6);
        }
        if (i > 0) {
            double difference = gsl_vector_get(pressures, i) - gsl_vector_get(pressures, i - 1);
            decrease_penalty += difference < 0.0 ? pow(difference, 2) : 0.0;
        }
    }

    // Additional penalty for having pressures decrease towards the tip
    gsl_vector_set(output_differences, raman->get_num_sample_points(), negative_penalty);

    // Add additional penalty for frequencies below zero
    gsl_vector_set(output_differences, raman->get_num_sample_points() + 1, decrease_penalty);

    return GSL_SUCCESS;    
}

void Fitting::callback(const size_t iter, void *params, 
              const gsl_multifit_nlinear_workspace *workspace) {
    int iteration_frequency;
    CallbackParams *info = ((CallbackParams *)params);
    gsl_vector *residual = gsl_multifit_nlinear_residual(workspace);
    gsl_vector *current_pressures = gsl_multifit_nlinear_position(workspace);
    std::vector<double> current_signal = info->sim_info->raman->get_raman_signal();

    if (info->print_freq != 0 && iter % info->print_freq == 0) {
        if (info->verbosity == 1) {
            std::cout << std::setw(10) << iter
                      << std::scientific << std::setprecision(10) << std::setw(20) << gsl_blas_dnrm2(residual)
                      << std::defaultfloat << std::setprecision(6) << std::endl;
        } else if (info->verbosity == 2) {
            std::cout << std::setw(10) << iter
                      << std::scientific << std::setprecision(10) << std::setw(20) << gsl_blas_dnrm2(residual)
                      << std::fixed << std::setprecision(2) << std::setw(14) << gsl_vector_max(current_pressures)
                      << std::setw(14) << gsl_vector_min(current_pressures)
                      << std::defaultfloat << std::setprecision(6) << std::endl;
        } else if (info->verbosity == 3) {
            std::cout << std::setw(10) << iter
                      << std::scientific << std::setprecision(10) << std::setw(20) << gsl_blas_dnrm2(residual)
                      << std::fixed << std::setprecision(2) << std::setw(14) << gsl_vector_max(current_pressures)
                      << std::setw(14) << gsl_vector_min(current_pressures)
                      << std::scientific << std::setprecision(5) << std::setw(18) << gsl_vector_get(residual, info->num_freqs)
                      << std::scientific << std::setprecision(5) << std::setw(23) << gsl_vector_get(residual, info->num_freqs + 1)
                      << std::defaultfloat << std::setprecision(6) << std::endl;
        }

        // Log pressures
        if (info->pressure_log.is_open()) {
            info->pressure_log << std::setprecision(0) << std::setw(6) <<  iter << "  ";
            for (int i = 0; i != current_pressures->size; i++) {
                info->pressure_log << std::fixed << std::setprecision(2) << std::setw(6) << gsl_vector_get(current_pressures, i);
            }
            info->pressure_log << "\n";
        }

        // Log frequencies
        if (info->signal_log.is_open()) {
            info->signal_log << std::setprecision(0) << std::setw(6) <<  iter << "  ";
            for (int i = 0; i != current_signal.size(); i++) {
                info->signal_log << std::scientific << std::setprecision(4) << std::setw(12) << current_signal[i];
            }
            info->signal_log << "\n";
        }
    }
}