#include <cmath>
#include <iostream>

#include "laser.h"

Laser::Laser(double intensity, double aperture,
             double wavelength, double ref_index,
             double focus_depth, int num_elements) : m_intensity(intensity),
                                   m_pinhole_num_aperture(aperture),
                                   m_wavelength(wavelength),
                                   m_lens_refractive_index(ref_index),
                                   m_z_focus_depth(focus_depth),
                                   m_z_intensity_profile(num_elements) {}

Laser::Laser(const Settings &settings) : m_intensity(settings.laser.intensity),
                                         m_pinhole_num_aperture(settings.laser.pinhole_num_aperture),
                                         m_wavelength(settings.laser.wavelength),
                                         m_lens_refractive_index(settings.laser.lens_refractive_index),
                                         m_z_focus_depth(settings.laser.z_focus_depth),
                                         m_z_intensity_profile(settings.diamond.num_elements) {

    set_z_intensity_profile(settings.diamond.depth, settings.diamond.num_elements);
}

void Laser::set_z_intensity_profile(double diamond_size, int num_elements) {
    double element_size = diamond_size / num_elements;
    double psf_width = 2 * m_wavelength * m_lens_refractive_index / pow(m_pinhole_num_aperture, 2);     // Point Spread Function (axial) width
    for (int i = 0; i != num_elements; i++) {
        double depth = i * element_size;
        m_z_intensity_profile[i] = exp(-pow((depth - m_z_focus_depth), 2) / (2 * pow(psf_width, 2)));
    }
}