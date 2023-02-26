#include <cmath>
#include <iostream>
#include "raman.h"

Raman::Raman(const int num_sampling_points, const int min_freq, const int max_freq) :
    m_sample_points(num_sampling_points),
    m_min_freq(min_freq),
    m_max_freq(max_freq),
    m_freq_range(max_freq - min_freq),
    m_spectrometer_resolution(static_cast<double>(m_freq_range) / static_cast<double>(m_sample_points)),
    m_raman_signal(m_sample_points, 0.0) {

}

std::vector<double> &Raman::get_raman_signal() {
    return m_raman_signal;
}

std::vector<double> &Raman::get_data_intensities() {
    return m_data_intensities;
}

void Raman::reset_raman_signal() {
    m_raman_signal = std::vector<double>(m_sample_points, 0.0);
}

double Raman::get_frequency(const double pressure) {
    // From EnkovichBLK16 (12C curve)
    return -5.9e-3 * std::pow(pressure, 2) + 2.91 * pressure + 1332.3;
}

double Raman::get_linewidth(const double pressure) {
    return 8.0;
}

void Raman::add_hydrostatic_signal(const double peak_intensity, const double peak_frequency, const double linewidth) {
    double frequency;
    for (int i = 0; i != m_sample_points; i++) {
        frequency = m_min_freq + (i * m_spectrometer_resolution);
        // Lorentzian distribution
        m_raman_signal[i] += peak_intensity * (1 / M_PI) *
                    (linewidth / (std::pow(frequency - peak_frequency, 2) + std::pow(linewidth, 2)));
    }
}

void Raman::compute_raman_signal(Diamond &diamond, Laser &laser) {
    double frequency, linewidth, intensity;

    reset_raman_signal();
    for (int i = 0; i != diamond.m_num_elements; i++) {
        frequency = get_frequency(diamond.m_pressure_profile[i]);
        linewidth = get_linewidth(diamond.m_pressure_profile[i]);
        intensity = diamond.get_attenuation(laser.m_intensity, 2 * i * diamond.m_element_size);
        add_hydrostatic_signal(intensity, frequency, linewidth);
    }
}

void Raman::write_signal(std::string &output_file) {
    std::ofstream output(output_file);
    output << "# Frequency (cm^-1)    Intensity" << std::endl;

    double frequency;
    for (int i = 0; i != m_sample_points; i++) {
        frequency = m_min_freq + (i * m_spectrometer_resolution);
        output << frequency << "    " << m_raman_signal[i] << "\n";
    }
    output << std::endl;
    output.close();
}

void Raman::read_signal(std::string &input_file) {
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
