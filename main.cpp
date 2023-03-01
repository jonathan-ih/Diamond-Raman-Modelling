#include <iostream>
#include <string>
#include <cmath>

#include "diamond.h"
#include "raman.h"
#include "laser.h"
#include "fitting.h"
#include "settings.h"

int main(int argc, char *argv[]) {

    std::string input_file(argv[1]);
    Settings settings(input_file);

    // Diamond parameters
    const int num_elements = settings.diamond.num_elements;
    const double depth = settings.diamond.depth;
    const double tip_pressure = settings.diamond.tip_pressure;

    // Raman parameters
    const int num_sample_points = settings.raman.num_sample_points;
    const double max_freq = settings.raman.max_freq;
    const double min_freq = settings.raman.min_freq;

    // Laser parameters
    const double intensity = settings.laser.intensity;

    // General parameters
    const int max_iter = settings.general.max_iter;
    std::string signal_output_file = settings.general.signal_output_file;
    std::string signal_input_file = settings.general.signal_input_file;
    std::string pressure_output_file = settings.general.pressure_output_file;
    std::string fitted_signal_file = settings.general.fitted_signal_file;
    std::string fitted_pressure_file = settings.general.fitted_pressure_file;

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

    /*********************
     *  Perform Fitting  *
     *********************/

    std::vector<double> new_pressure_profile;
    std::vector<double> starting_pressures;
    // Initialise to linear profile  
    for (int i = 0; i != diamond.m_num_elements; i++) {
        starting_pressures.push_back(i * (tip_pressure / diamond.m_num_elements));
    }

    Fitting fitting(raman, diamond, laser);

    fitting.set_initial_pressures(starting_pressures);
    fitting.initialize();
    fitting.fit(max_iter);
    fitting.print_summary();
    new_pressure_profile = fitting.get_new_pressure_profile();

    diamond.set_pressure_profile(new_pressure_profile);
    raman.compute_raman_signal(diamond, laser);

    raman.write_signal(fitted_signal_file);
    diamond.write_pressure(fitted_pressure_file);
}
