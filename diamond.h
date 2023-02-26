#ifndef DIAMOND_RAMAN_MODELLING_DIAMOND_H
#define DIAMOND_RAMAN_MODELLING_DIAMOND_H

#include <vector>
#include <string>

class Diamond {
public:
    double m_depth;
    int m_num_elements;
    double m_element_size;
    std::vector<double> m_pressure_profile;

    static constexpr double penetration_depth = 100.0;

    Diamond(double depth, int num_elements);

    double get_attenuation(double initial_intensity, double distance) const;
    void set_pressure_profile(const std::vector<double> pressure_profile);
    void write_pressure(const std::string &output_file);
};


#endif //DIAMOND_RAMAN_MODELLING_DIAMOND_H
