#ifndef DIAMOND_RAMAN_MODELLING_DIAMOND_H
#define DIAMOND_RAMAN_MODELLING_DIAMOND_H

#include <vector>
#include <string>

#include "settings.h"

class Diamond {
    static constexpr double penetration_depth = 100.0;
public:

    Diamond(double depth, int num_elements);
    Diamond(const DiamondSettings &diamond_settings);

    double get_depth() const { return m_depth; }
    int get_num_elements() const { return m_num_elements; }
    double get_element_size() const { return m_element_size; }
    std::vector<double> &get_pressure_profile() { return m_pressure_profile; }
    const std::vector<double> &get_pressure_profile() const {return m_pressure_profile; }

    double get_attenuation(double initial_intensity, double distance);
    double get_attenuation(double initial_intensity, double distance) const;
    void set_pressure_profile(const std::vector<double> &pressure_profile);
    void set_pressure_profile(const std::string &pressure_profile);
    void write_pressure(const std::string &output_file);

private:
    double m_depth;
    int m_num_elements;
    double m_element_size;
    std::vector<double> m_pressure_profile;

    void set_linear_profile(const double tip_pressure);
    void set_quadratic_profile(const double tip_pressure);
    void set_file_profile(const std::string &input_file);
};


#endif //DIAMOND_RAMAN_MODELLING_DIAMOND_H
