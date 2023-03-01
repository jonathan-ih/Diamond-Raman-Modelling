#ifndef DIAMOND_RAMAN_MODELLING_SETTINGS_H
#define DIAMOND_RAMAN_MODELLING_SETTINGS_H

#include <vector>
#include <string>
#include <map>
#include <set>

enum SettingType{
    INTEGER,
    POSITIVE_INTEGER,
    NEGATIVE_INTEGER,
    FLOAT,
    POSITIVE_FLOAT,
    NEGATIVE_FLOAT,
    TEXT,
};

struct SettingInfo {
    SettingType setting_type;
    std::set<std::string> allowed_values;
    std::string default_value;
    bool required;
    void* assignment_pointer;
};

struct DiamondSettings {
    int num_elements;
    double depth;
    double tip_pressure;
    std::string initial_profile;
};

struct RamanSettings {
    int num_sample_points;
    double max_freq;
    double min_freq;
};

struct LaserSettings {
    double intensity;
};

struct GeneralSettings {
    int max_iter;
    std::string signal_output_file;
    std::string signal_input_file;
    std::string pressure_output_file;
    std::string fitted_signal_file;
    std::string fitted_pressure_file;
};

class Settings {
public:
    DiamondSettings diamond;
    RamanSettings raman;
    LaserSettings laser;
    GeneralSettings general;

    Settings(std::string &input_file);
private:
    std::vector<std::string> read_input_file(const std::string &input_file);
    void clean_input_file(std::vector<std::string> &file_contents);
    void process_input_file(std::vector<std::string> &file_contents);
    void process_section(std::string section, std::vector<std::string> section_contents);
    void validate_and_assign(std::string key, SettingInfo info);
    void assign_values_to_structs(std::string section, std::map<std::string, void*> setting_values);


    std::map<std::string, SettingInfo> diamond_settings_info = {
        {"NELEM", {POSITIVE_INTEGER, {}, "100", false, &diamond.num_elements}},
        {"DEPTH", {POSITIVE_FLOAT, {}, "0", true, &diamond.depth}},
        {"INITIAL_PROFILE", {TEXT, {"LINEAR", "QUADRATIC"},"LINEAR", false, &diamond.initial_profile}},
        {"TIP_PRESSURE", {POSITIVE_FLOAT, {}, "0", false, &diamond.tip_pressure}},
    };
    std::map<std::string, SettingInfo> raman_settings_info = {
        {"NFREQ", {POSITIVE_INTEGER, {}, "1000", false, &raman.num_sample_points}},
        {"MAX_FREQ", {POSITIVE_FLOAT, {}, "1500", false, &raman.max_freq}},
        {"MIN_FREQ", {POSITIVE_FLOAT, {}, "1000", false, &raman.min_freq}},
    };
    std::map<std::string, SettingInfo> laser_settings_info = {
        {"INTENSITY", {POSITIVE_FLOAT, {}, "100", false, &laser.intensity}},
    };
    std::map<std::string, SettingInfo> general_settings_info = {
        {"MAX_ITER", {POSITIVE_INTEGER, {}, "100", false, &general.max_iter}},
        {"SIG_IN", {TEXT, {}, "signal.in", false, &general.signal_input_file}},
        {"SIG_OUT", {TEXT, {}, "signal.out", false, &general.signal_output_file}},
        {"PRESS_OUT", {TEXT, {}, "pressure.out", false, &general.pressure_output_file}},
        {"FIT_SIG_OUT", {TEXT, {}, "fitted_signal.out", false, &general.fitted_signal_file}},
        {"FIT_PRESS_OUT", {TEXT, {}, "fitted_pressure.out", false, &general.fitted_pressure_file}},
    };
};

#endif // DIAMOND_RAMAN_MODELLING_SETTINGS_H