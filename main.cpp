#include <iostream>
#include <string>
#include <cmath>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_multifit_nlinear.h>

#include "diamond.h"
#include "raman.h"

struct FittingData {
    Raman *raman;
    Diamond *diamond;
    Laser *laser;
};

int compute_fitting_residuals(const gsl_vector *pressures, void *data,
                                 gsl_vector *output_differences) {
    // Cast pointer to void to pointer to struct and extract the member variables
    Raman raman = *((struct FittingData *)data)->raman;
    Diamond diamond = *((struct FittingData *)data)->diamond;
    Laser laser = *((struct FittingData *)data)->laser;

    std::vector<double> pressure_profile(pressures->size);
    for (int i = 0; i != pressures->size; i++) {
        pressure_profile[i] = gsl_vector_get(pressures, i);
    }

    diamond.set_pressure_profile(pressure_profile);
    raman.compute_raman_signal(diamond, laser);

    std::vector<double> actual = raman.get_data_intensities();
    std::vector<double> predicted = raman.get_raman_signal();
    for (int i = 0; i != raman.m_sample_points; i++) {
        gsl_vector_set(output_differences, i, predicted[i] - actual[i]);
    }

    return GSL_SUCCESS;    
}

void callback(const size_t iter, void *pressures, 
              const gsl_multifit_nlinear_workspace *workspace) {
    // gsl_vector *residual = gsl_multifit_nlinear_residual(workspace);
    // gsl_vector *current_pressures = gsl_multifit_nlinear_position(workspace);
    std::cout << "Iteration " << iter << "\n";
}

int main(int argc, char *argv[]) {

    // Diamond parameters
    const int num_elements = 10;
    const double depth = 10.0;
    const double tip_pressure = 50;

    // Raman parameters
    const int num_sample_points = 1000;
    const int max_freq = 1500;
    const int min_freq = 1000;

    // Laser parameters
    const double intensity = 100;

    // General parameters
    std::string signal_output_file = "signal.out";
    std::string signal_input_file = "signal.out";
    std::string pressure_output_file = "pressure.out";
    std::string fitted_signal_file = "fitted_signal.out";
    std::string fitted_pressure_file = "fitted_pressure.out";

    std::vector<double> pressure_profile(num_elements);

    Diamond diamond(depth, num_elements);
    Laser laser(intensity);
    Raman raman(num_sample_points, min_freq, max_freq);

    // Quadratic pressure profile
    for (int i = 0; i != num_elements; i++) {
        pressure_profile[i] = pow(i * (sqrt(tip_pressure) / num_elements), 2);
    }
    diamond.set_pressure_profile(pressure_profile);
    diamond.write_pressure(pressure_output_file);

    raman.compute_raman_signal(diamond, laser);
    raman.write_signal(signal_output_file);

    raman.read_signal(signal_input_file);

    // Fit the data
    // Fit the intensities to the frequencies with the pressures in the DAC as parameters
    // Trust region type of fitting
    const gsl_multifit_nlinear_type *fittingtype = gsl_multifit_nlinear_trust;

    // Define workspace that holds variables (matrices and vectors) needed for fitting
    gsl_multifit_nlinear_workspace *workspace;

    // Fitting equations holds function and derivative of function (fdf)
    // In this case derivative of the function will be calculated numerically
    gsl_multifit_nlinear_fdf fitting_equations;

    // Set some default parameters
    gsl_multifit_nlinear_parameters fitting_equations_params = 
        gsl_multifit_nlinear_default_parameters();
    FittingData params {&raman, &diamond, &laser};
    const int num_frequencies = raman.m_sample_points;
    const int num_pressures = diamond.m_num_elements;
    double *starting_pressures = new double[num_pressures]();       // Initialise to 0
    double *wts = new double[num_frequencies];        // weights
    for (int i = 0; i != num_frequencies; i++) {
        wts[i] = 1.0;
    }
    gsl_vector_view pressures = gsl_vector_view_array(starting_pressures, num_pressures);
    gsl_vector_view weights = gsl_vector_view_array(wts, num_frequencies);

    // Set tolerances
    const double xtol = 1e-8;
    const double gtol = 1e-8;
    const double ftol = 0.0;

    // Define function to be minimised
    fitting_equations.f = compute_fitting_residuals;
    fitting_equations.df = NULL;    // Compute Jacobian from finite difference
    fitting_equations.fvv = NULL;   // Do not use geodesic acceleration
    fitting_equations.n = num_frequencies;
    fitting_equations.p = num_pressures;
    fitting_equations.params = &params;

    // Define variables to track and analyse fitting
    gsl_vector *residuals;
    gsl_matrix *jacobian;
    gsl_matrix *covariance = gsl_matrix_alloc(num_pressures, num_pressures);
    double chisq, chisq0;
    int status, info;

    // Allocate the workspace with default parameters
    workspace = gsl_multifit_nlinear_alloc(fittingtype, &fitting_equations_params, num_frequencies, num_pressures);

    // initialize solver with starting point and weights
    gsl_multifit_nlinear_winit(&pressures.vector, &weights.vector, &fitting_equations, workspace);

    // compute initial cost function
    residuals = gsl_multifit_nlinear_residual(workspace);
    gsl_blas_ddot(residuals, residuals, &chisq0);

    // solve the system with a maximum of 100 iterations
    status = gsl_multifit_nlinear_driver(100, xtol, gtol, ftol, callback, NULL, &info, workspace);

    // compute covariance of best fit parameters
    jacobian = gsl_multifit_nlinear_jac(workspace);
    gsl_multifit_nlinear_covar(jacobian, 0.0, covariance);

    // compute final cost
    gsl_blas_ddot(residuals, residuals, &chisq);

    // Print summary of fitting
#define FIT(i) gsl_vector_get(workspace->x, i)
#define ERR(i) sqrt(gsl_matrix_get(covariance,i,i))

    std::cout << "summary from method '" << gsl_multifit_nlinear_name(workspace)
              << "/" << gsl_multifit_nlinear_trs_name(workspace) << "'\n";
    std::cout << "number of iterations: " << gsl_multifit_nlinear_niter(workspace) << "\n";
    std::cout << "function evaluations: " << fitting_equations.nevalf << "\n";
    std::cout << "Jacobian evaluations: " << fitting_equations.nevaldf << "\n";
    std::cout << "reason for stopping: " << ((info == 1) ? "small step size" : "small gradient") << "\n";
    std::cout << "initial |f(x)| = " << sqrt(chisq0) << "\n";
    std::cout << "final   |f(x)| = " << sqrt(chisq) << std::endl;

    std::cout << "Pressures\n";
    for (int i = 0; i != num_pressures; i++) {
        std::cout << "Initial: " << starting_pressures[i] 
                  << "\tCurrent: " << FIT(i) << "\n";
    }
    std::cout << std::endl;

    std::vector<double> new_pressure_profile(num_pressures);
    for (int i = 0; i != num_pressures; i++) {
        new_pressure_profile[i] = gsl_vector_get(workspace->x, i);
    }

    diamond.set_pressure_profile(new_pressure_profile);
    raman.compute_raman_signal(diamond, laser);

    raman.write_signal(fitted_signal_file);
    diamond.write_pressure(fitted_pressure_file);

    // Free memory
    gsl_multifit_nlinear_free(workspace);
    gsl_matrix_free(covariance);
    delete [] starting_pressures;
    delete [] wts;
}
