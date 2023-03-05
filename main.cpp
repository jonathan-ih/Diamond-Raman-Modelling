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
        raman.read_signal(signal_input_file);

        Fitting fitting(settings, raman, diamond, laser);

        fitting.initialize();
        fitting.fit();
        fitting.print_summary();

        raman.write_signal(signal_output_file);
        diamond.write_pressure(pressure_output_file);
    }
}
