#ifndef DIAMOND_RAMAN_MODELLING_RAMAN_H
#define DIAMOND_RAMAN_MODELLING_RAMAN_H

#include <vector>
#include <string>
#include <fstream>

#include "diamond.h"
#include "laser.h"

class Raman {
public:
    int m_min_freq;
    int m_max_freq;
    int m_sample_points;

    Raman(int num_sampling_points, int min_freq, int max_freq);

    static double get_frequency(double pressure);
    static double get_linewidth(double pressure);

    void add_hydrostatic_signal(double peak_intensity, double peak_frequency, double linewidth);
    void compute_raman_signal(Diamond &diamond, Laser &laser);
    void reset_raman_signal();

    std::vector<double> &get_raman_signal();
    std::vector<double> &get_data_intensities();
    void write_signal(std::string &output_file);
    void read_signal(std::string &input_file);

private:
    int m_freq_range;
    double m_spectrometer_resolution;
    std::vector<double> m_raman_signal;
    std::vector<double> m_data_frequencies;
    std::vector<double> m_data_intensities;

};


#endif //DIAMOND_RAMAN_MODELLING_RAMAN_H
