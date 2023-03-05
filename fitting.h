#ifndef DIAMOND_RAMAN_MODELLING_FITTING_H
#define DIAMOND_RAMAN_MODELLING_FITTING_H

#include <vector>

#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_multifit_nlinear.h>

#include "settings.h"
#include "diamond.h"
#include "raman.h"
#include "laser.h"

struct SimulationInfo {
    Raman *raman;
    Diamond *diamond;
    Laser *laser;
};

struct CallbackParams {
    int verbosity;
    int max_iter;
    int print_freq;
    int num_freqs;
    int num_pressures;
    std::ofstream pressure_log;
    std::ofstream signal_log;
    SimulationInfo *sim_info;
};

class Fitting {
    static int compute_cost_function(const gsl_vector *pressures, void *data, gsl_vector *output_differences);
    static void callback(const size_t iter, void *params,  const gsl_multifit_nlinear_workspace *workspace);
public:

    Fitting(const Settings &settings, Raman &raman, Diamond &diamond, Laser &laser);
    ~Fitting();
    
    void set_initial_pressures(const std::vector<double> &init_pressures);
    void initialize();
    void fit();
    void print_summary() const;

private:
    int m_num_frequencies;
    int m_num_pressures;
    double *m_starting_pressures;
    double *m_data_weights;
    double m_chisq, m_chisq0;
    int m_verbosity;
    int m_max_iter;
    int m_print_freq;
    std::string m_pressure_log;
    std::string m_signal_log;

    // Set tolerances
    double m_xtol;
    double m_gtol;
    double m_ftol = 0.0;        // Not implemented (would be small residual tolerance)

    // Number of additional constraints
    int m_num_constraints = 2;    


    // Trust region type of fitting (only type available for non-linear)
    const gsl_multifit_nlinear_type *m_fittingtype = gsl_multifit_nlinear_trust;

    // Define workspace that holds variables (matrices and vectors) needed for fitting
    gsl_multifit_nlinear_workspace *m_workspace;

    // Fitting equations holds function and derivative of function (fdf)
    // In this case derivative of the function will be calculated numerically
    gsl_multifit_nlinear_fdf m_fitting_equations;

    gsl_multifit_nlinear_parameters m_fitting_params;   // Parameters for the fitter (tolerances etc.)
    SimulationInfo m_simulation_info;                      // Information on the simulation (pointers to relevant Raman, Diamond, Laser)
    CallbackParams m_callback_params;

    // Define variables to track and analyse fitting
    gsl_vector *m_residuals;
    gsl_matrix *m_jacobian;
    gsl_matrix *m_covariance;
    int m_status, m_info;

    gsl_vector_view m_pressures;
    gsl_vector_view m_weights;

    void print_fitting_header() const;
};

#endif //DIAMOND_RAMAN_MODELLING_FITTING_H