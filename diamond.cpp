#include <cmath>
#include <fstream>

#include "diamond.h"

Diamond::Diamond(const double depth, const int num_elements) : m_depth(depth), m_num_elements(num_elements),
                                                               m_element_size(m_depth / m_num_elements),
                                                               m_pressure_profile(m_num_elements) {}

Diamond::Diamond(const DiamondSettings &diamond_settings) : m_depth(diamond_settings.depth),
                                                            m_num_elements(diamond_settings.num_elements),
                                                            m_element_size(m_depth / m_num_elements),
                                                            m_pressure_profile(m_num_elements) {
    if (diamond_settings.pressure_profile == "LINEAR") {
        set_linear_profile(diamond_settings.tip_pressure);
    } else if (diamond_settings.pressure_profile == "QUADRATIC") {
        set_quadratic_profile(diamond_settings.tip_pressure);
    }
}

void Diamond::set_pressure_profile(const std::vector<double> &pressure_profile) {
    for (int i = 0; i != m_num_elements; i++) {
        m_pressure_profile[i] = pressure_profile[i];
    }
}

void Diamond::set_pressure_profile(const std::string &pressure_profile) {
    set_file_profile(pressure_profile);
}

double Diamond::get_attenuation(double initial_intensity, double distance) {
    return initial_intensity * exp(-distance / penetration_depth);
}

double Diamond::get_attenuation(double initial_intensity, double distance) const {
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

void Diamond::set_linear_profile(double tip_pressure) {
    for (int i = 0; i != m_num_elements; i++) {
        m_pressure_profile[i] = i * (tip_pressure / m_num_elements);
    }
}

void Diamond::set_quadratic_profile(double tip_pressure) {
    for (int i = 0; i != m_num_elements; i++) {
        m_pressure_profile[i] = pow(i * (sqrt(tip_pressure) / m_num_elements), 2);
    }
}

void Diamond::set_file_profile(const std::string &input_file) {
    std::ifstream input(input_file);
    std::string line;
    m_pressure_profile.clear();

    double depth, pressure;

    // Read first line
    std::getline(input, line);

    while (input >> depth >> pressure) {
        m_pressure_profile.push_back(pressure);
    }

    if (m_pressure_profile.size() != m_num_elements) {
        throw std::runtime_error("File " + input_file + " does not have the correct length");
    }
}