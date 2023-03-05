#include <cmath>
#include <iostream>
#include "raman.h"

Raman::Raman(int num_sampling_points, double min_freq, double max_freq) :
    m_num_sample_points(num_sampling_points),
    m_min_freq(min_freq),
    m_max_freq(max_freq),
    m_freq_range(m_max_freq - m_min_freq),
    m_spectrometer_resolution(m_freq_range / static_cast<double>(m_num_sample_points)),
    m_raman_signal(m_num_sample_points, 0.0) {}

Raman::Raman(const Settings &settings) : 
    m_num_sample_points(settings.raman.num_sample_points),
    m_min_freq(settings.raman.min_freq),
    m_max_freq(settings.raman.max_freq),
    m_freq_range(m_max_freq - m_min_freq),
    m_spectrometer_resolution(m_freq_range / static_cast<double>(m_num_sample_points)),
    m_raman_signal(m_num_sample_points, 0.0) {}

void Raman::reset_raman_signal() {
    m_raman_signal = std::vector<double>(m_num_sample_points, 0.0);
}

double Raman::compute_frequency(double pressure) {
    // From EnkovichBLK16 (12C curve)
    return -5.9e-3 * std::pow(pressure, 2) + 2.91 * pressure + 1332.3;
}

double Raman::compute_linewidth(double pressure) {
    return 8.0;
}

void Raman::add_hydrostatic_signal(double peak_intensity, double peak_frequency, double linewidth) {
    double frequency;
    for (int i = 0; i != m_num_sample_points; i++) {
        frequency = m_min_freq + (i * m_spectrometer_resolution);
        // Lorentzian distribution
        m_raman_signal[i] += peak_intensity * (1 / M_PI) *
                    (linewidth / (std::pow(frequency - peak_frequency, 2) + std::pow(linewidth, 2)));
    }
}

void Raman::compute_raman_signal(const Diamond &diamond, const Laser &laser) {
    double frequency, linewidth, intensity;

    reset_raman_signal();
    for (int i = 0; i != diamond.get_num_elements(); i++) {
        frequency = compute_frequency(diamond.get_pressure_profile()[i]);
        linewidth = compute_linewidth(diamond.get_pressure_profile()[i]);
        intensity = diamond.get_attenuation(laser.get_intensity(), 2 * i * diamond.get_element_size());
        add_hydrostatic_signal(intensity, frequency, linewidth);
    }
}

void Raman::write_signal(const std::string &output_file) const {
    std::ofstream output(output_file);
    output << "# Frequency (cm^-1)    Intensity" << std::endl;

    double frequency;
    for (int i = 0; i != m_num_sample_points; i++) {
        frequency = m_min_freq + (i * m_spectrometer_resolution);
        output << frequency << "    " << m_raman_signal[i] << "\n";
    }
    output << std::endl;
    output.close();
}


void Raman::read_signal(const std::string &input_file) {
    std::ifstream input(input_file);
    std::string line;

    double frequency, intensity;

    // Read first line
    std::getline(input, line);

    while (input >> frequency >> intensity) {
        m_data_frequencies.push_back(frequency);
        m_data_intensities.push_back(intensity);
    }
}
