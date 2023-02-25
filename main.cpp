#include <iostream>
#include <string>
#include <cmath>

#include "diamond.h"
#include "raman.h"

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
    std::string output_file = "signal.out";
    std::string input_file = "signal.in";

    std::vector<double> pressure_profile(num_elements);

    Diamond diamond(depth, num_elements);
    Laser laser(intensity);
    Raman raman(num_sample_points, min_freq, max_freq);

    // Quadratic pressure profile
    for (int i = 0; i != num_elements; i++) {
        pressure_profile[i] = pow(i * (sqrt(tip_pressure) / num_elements), 2);
    }
    diamond.set_pressure_profile(pressure_profile);

    raman.compute_raman_signal(diamond, laser);
    raman.write_signal(output_file);

    raman.read_signal(input_file);

    // Fit the data
    // Fit the intensities to the frequencies with the pressures in the DAC as parameters

}
