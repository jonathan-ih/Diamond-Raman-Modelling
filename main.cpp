#include <iostream>
#include <string>
#include <cmath>

#include "diamond.h"
#include "raman.h"
#include "laser.h"
#include "fitting.h"

int main(int argc, char *argv[]) {

    // Diamond parameters
    const int num_elements = 100;
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
    fitting.fit(100);
    fitting.print_summary();
    new_pressure_profile = fitting.get_new_pressure_profile();

    diamond.set_pressure_profile(new_pressure_profile);
    raman.compute_raman_signal(diamond, laser);

    raman.write_signal(fitted_signal_file);
    diamond.write_pressure(fitted_pressure_file);
}
