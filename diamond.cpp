#include <cmath>
#include <fstream>

#include "diamond.h"

Diamond::Diamond(const double depth, const int num_elements) : m_depth(depth), m_num_elements(num_elements),
                                                               m_element_size(depth / num_elements),
                                                               m_pressure_profile(num_elements) {}

void Diamond::set_pressure_profile(const std::vector<double> pressure_profile) {
    for (int i = 0; i != m_num_elements; i++) {
        m_pressure_profile[i] = pressure_profile[i];
    }
}

double Diamond::get_attenuation(const double initial_intensity, const double distance) const {
    return initial_intensity * exp(-distance / penetration_depth);
}

void Diamond::write_pressure(const std::string &output_file) {
    std::ofstream output(output_file);
    output << "# Distance (mm)    Pressure (GPa)" << std::endl;

    double distance;
    for (int i = 0; i != m_num_elements; i++) {
        distance = i * m_element_size;
        output << distance << "    " << m_pressure_profile[i] << "\n";
    }
    output << std::endl;

    output.close();
}
