#ifndef DIAMOND_RAMAN_MODELLING_LASER_H
#define DIAMOND_RAMAN_MODELLING_LASER_H

#include "settings.h"

class Laser {
public:

    Laser(double intensity, double aperture, double wavelength, 
          double ref_index, double focus_depth, int num_elements);
    Laser(const Settings &settings);

    double get_intensity() { return m_intensity; }
    double get_intensity() const { return m_intensity; }
    std::vector<double> &get_z_intensity_profile() { return m_z_intensity_profile; }
    const std::vector<double> &get_z_intensity_profile() const { return m_z_intensity_profile; }


private:
    double m_intensity;
    double m_pinhole_num_aperture;
    double m_wavelength;
    double m_lens_refractive_index;
    double m_z_focus_depth;

    std::vector<double> m_z_intensity_profile;

    void set_z_intensity_profile(double diamond_size, int num_elements);

};


#endif //DIAMOND_RAMAN_MODELLING_LASER_H
