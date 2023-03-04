#ifndef DIAMOND_RAMAN_MODELLING_RAMAN_H
#define DIAMOND_RAMAN_MODELLING_RAMAN_H

#include <vector>
#include <string>
#include <fstream>

#include "diamond.h"
#include "laser.h"
#include "settings.h"

class Raman {
    static double compute_frequency(double pressure);
    static double compute_linewidth(double pressure);

public:
    Raman(int num_sampling_points, double min_freq, double max_freq);
    Raman(const RamanSettings &raman_settings);

    void add_hydrostatic_signal(double peak_intensity, double peak_frequency, double linewidth);
    void compute_raman_signal(const Diamond &diamond, const Laser &laser);
    void reset_raman_signal();

    double get_min_freq() const { return m_min_freq; }
    double get_max_freq() const { return m_max_freq; }
    int get_num_sample_points() const {return m_num_sample_points; }
    std::vector<double> &get_raman_signal() { return m_raman_signal; }
    std::vector<double> &get_data_intensities() {return m_data_intensities; }
    const std::vector<double> &get_raman_signal() const { return m_raman_signal; }
    const std::vector<double> &get_data_intensities() const { return m_data_intensities; }
    void write_signal(const std::string &output_file) const;
    void read_signal(const std::string &input_file);

private:
    double m_min_freq;
    double m_max_freq;
    int m_num_sample_points;
    int m_freq_range;
    double m_spectrometer_resolution;
    std::vector<double> m_raman_signal;
    std::vector<double> m_data_frequencies;
    std::vector<double> m_data_intensities;

};


#endif //DIAMOND_RAMAN_MODELLING_RAMAN_H
