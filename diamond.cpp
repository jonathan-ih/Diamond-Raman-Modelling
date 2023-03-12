#include <cmath>
#include <fstream>

#include "diamond.h"

Diamond::Diamond(double depth, int num_elements, double penetration_depth) :
    m_depth(depth), m_num_elements(num_elements),
    m_element_size(m_depth / m_num_elements),
    m_pressure_profile(m_num_elements),
    m_penetration_depth(penetration_depth) {}

Diamond::Diamond(const Settings &settings) : m_depth(settings.diamond.depth),
                                                            m_num_elements(settings.diamond.num_elements),
                                                            m_element_size(m_depth / m_num_elements),
                                                            m_pressure_profile(m_num_elements),
                                                            m_penetration_depth(settings.diamond.penetration_depth) {
    if (settings.diamond.pressure_profile == "LINEAR") {
        set_linear_profile(settings.diamond.tip_pressure);
    } else if (settings.diamond.pressure_profile == "QUADRATIC") {
        set_quadratic_profile(settings.diamond.tip_pressure);
    } else if (settings.diamond.pressure_profile == "FILE") {
        set_pressure_profile(settings.general.pressure_input_file);
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
    return initial_intensity * exp(-distance / m_penetration_depth);
}

double Diamond::get_attenuation(double initial_intensity, double distance) const {
    return initial_intensity * exp(-distance / m_penetration_depth);
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