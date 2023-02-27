#ifndef DIAMOND_RAMAN_MODELLING_FITTING_H
#define DIAMOND_RAMAN_MODELLING_FITTING_H

#include <vector>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_multifit_nlinear.h>

#include "diamond.h"
#include "raman.h"
#include "laser.h"

struct FittingData {
    Raman *raman;
    Diamond *diamond;
    Laser *laser;
};

class Fitting {
public:
    const int m_num_frequencies;
    const int m_num_pressures;
    double *m_starting_pressures;
    double *m_data_weights;
    double m_chisq, m_chisq0;

    Fitting(Raman &raman, Diamond &diamond, Laser &laser);
    ~Fitting();
    
    void set_initial_pressures(std::vector<double> init_pressures);
    void initialize();
    void fit(const int max_iter);
    void print_summary();
    std::vector<double> get_new_pressure_profile();

private:
    // Trust region type of fitting (only type available for non-linear)
    const gsl_multifit_nlinear_type *m_fittingtype = gsl_multifit_nlinear_trust;

    // Define workspace that holds variables (matrices and vectors) needed for fitting
    gsl_multifit_nlinear_workspace *m_workspace;

    // Fitting equations holds function and derivative of function (fdf)
    // In this case derivative of the function will be calculated numerically
    gsl_multifit_nlinear_fdf m_fitting_equations;

    gsl_multifit_nlinear_parameters m_fitting_equations_params;
    FittingData m_params;

    // Set tolerances
    const double m_xtol = 1e-8;
    const double m_gtol = 1e-8;
    const double m_ftol = 0.0;

    // Define variables to track and analyse fitting
    gsl_vector *m_residuals;
    gsl_matrix *m_jacobian;
    gsl_matrix *m_covariance;
    int m_status, m_info;

    const int m_num_constraints = 2;

    gsl_vector_view m_pressures;
    gsl_vector_view m_weights;

    static int compute_cost_function(const gsl_vector *pressures, void *data,
                                     gsl_vector *output_differences);
    static void callback(const size_t iter, void *pressures, 
                         const gsl_multifit_nlinear_workspace *workspace);
};

#endif //DIAMOND_RAMAN_MODELLING_FITTING_H