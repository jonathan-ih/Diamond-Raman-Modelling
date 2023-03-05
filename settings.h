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
    std::string pressure_profile;
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
    std::string mode;
    int max_iter;
    int verbosity;
    std::string signal_output_file;
    std::string signal_input_file;
    std::string pressure_input_file;
    std::string pressure_output_file;
};

class Settings {
public:
    DiamondSettings diamond;
    RamanSettings raman;
    LaserSettings laser;
    GeneralSettings general;

    Settings(const std::string &input_file);

private:
    std::vector<std::string> read_input_file(const std::string &input_file);
    void clean_file_contents(std::vector<std::string> &file_contents);
    void process_input_file(const std::vector<std::string> &file_contents);
    void process_section(const std::string &section, const std::vector<std::string> &section_contents);
    void validate_and_assign(const std::string &key, const SettingInfo &info);
    void check_allowed_values(const std::string &value_string, const std::set<std::string> &allowed_values);

    std::map<std::string, SettingInfo> diamond_settings_info = {
        {"NELEM", {POSITIVE_INTEGER, {}, "100", false, &diamond.num_elements}},
        {"DEPTH", {POSITIVE_FLOAT, {}, "0", true, &diamond.depth}},
        {"PRESSURE_PROFILE", {TEXT, {"LINEAR", "QUADRATIC", "FILE"},"LINEAR", false, &diamond.pressure_profile}},
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
        {"MODE", {TEXT, {"SIMULATE", "FIT"}, "", true, &general.mode}},
        {"MAX_ITER", {POSITIVE_INTEGER, {}, "100", false, &general.max_iter}},
        {"VERBOSITY", {POSITIVE_INTEGER, {"0", "1", "2", "3"}, "1", false, &general.verbosity}},
        {"SIG_IN", {TEXT, {}, "signal.in", false, &general.signal_input_file}},
        {"SIG_OUT", {TEXT, {}, "signal.out", false, &general.signal_output_file}},
        {"PRESS_IN", {TEXT, {}, "pressure.in", false, &general.pressure_input_file}},
        {"PRESS_OUT", {TEXT, {}, "pressure.out", false, &general.pressure_output_file}},
    };
};

#endif // DIAMOND_RAMAN_MODELLING_SETTINGS_H