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
    const Settings settings(input_file);

    Diamond diamond(settings);
    Raman raman(settings);
    Laser laser(settings);

    // General parameters
    const int max_iter = settings.general.max_iter;
    std::string signal_output_file = settings.general.signal_output_file;
    std::string signal_input_file = settings.general.signal_input_file;
    std::string pressure_output_file = settings.general.pressure_output_file;
    std::string pressure_input_file = settings.general.pressure_input_file;

    if (settings.general.mode == "SIMULATE") {
        diamond.write_pressure(pressure_output_file);
        raman.compute_raman_signal(diamond, laser);
        raman.write_signal(signal_output_file);
    } else if (settings.general.mode == "FIT") {
        std::vector<double> new_pressure_profile;

        raman.read_signal(signal_input_file);

        Fitting fitting(raman, diamond, laser);

        fitting.initialize();
        fitting.fit(max_iter);
        fitting.print_summary();

        // Shouldn't have to do this
        new_pressure_profile = fitting.get_new_pressure_profile();

        diamond.set_pressure_profile(new_pressure_profile);
        raman.compute_raman_signal(diamond, laser);

        raman.write_signal(signal_output_file);
        diamond.write_pressure(pressure_output_file);
    }
}
