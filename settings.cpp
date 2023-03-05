#include <iostream>
#include <fstream>
#include <cctype>
#include <algorithm>
#include <stdexcept>

#include "settings.h"

Settings::Settings(const std::string &input_file) {
    std::vector<std::string> file_contents = read_input_file(input_file);
    clean_file_contents(file_contents);
    process_input_file(file_contents);
}

std::vector<std::string> Settings::read_input_file(const std::string &input_file) {
    std::ifstream input(input_file);
    std::string line;
    std::vector<std::string> file_contents;
    while(std::getline(input, line)) {
        file_contents.push_back(line);
    }
    return file_contents;
}

void Settings::process_input_file(const std::vector<std::string> &file_contents) {
    std::string current_section;
    std::vector<std::string> section_contents;

    for (int i = 0; i != file_contents.size(); i++) {
        std::string line = file_contents[i];

        // Start section
        if (line[0] == '&') {
            current_section = line.substr(1, line.size());
        // End section
        } else if (line == "/") {
            process_section(current_section, section_contents);
            
            // Reset section variables
            section_contents.clear();
            current_section.clear();
        // During section
        } else {
            section_contents.push_back(line);
        }
    }
}

void Settings::clean_file_contents(std::vector<std::string> &file_contents) {
    auto it = file_contents.begin();
    while(it != file_contents.end()) {
        // Strip any trailing comments 
        *it = it->substr(0, it->find_first_of("#"));

        // Strip whitespace
        it->erase(std::remove_if(it->begin(), it->end(), ::isspace), it->end());
        
        // Remove empty lines
        if (it->empty()) {
            it = file_contents.erase(it);
        } else {
            it++;
        }
    }
}

void Settings::process_section(const std::string &section, const std::vector<std::string> &section_contents) {
    std::map<std::string, std::string> user_settings;
    for (auto &line : section_contents) {
        std::string key = line.substr(0, line.find("="));
        std::string value = line.substr(line.find("=")+1, line.size());
        user_settings[key] = value;
    }

    std::map<std::string, SettingInfo> settings_map;
    if (section == "DIAMOND") {
        settings_map = diamond_settings_info;
    } else if (section == "RAMAN") {
        settings_map = raman_settings_info;
    } else if (section == "LASER") {
        settings_map = laser_settings_info;
    } else if (section == "GENERAL") {
        settings_map = general_settings_info;
    } else {
        throw std::runtime_error("Section " + section + " not recognised"); 
    }

    for (auto &setting : settings_map) {
        std::string key = setting.first;
        SettingInfo info = setting.second;
        std::string value_string;
        bool is_required = info.required;

        // Check required settings are given
        if (is_required && user_settings.find(key) == user_settings.end()) {
            throw std::runtime_error("Required setting " + key + " in section &" 
                                     + section + " not found.\n");
        }

        // If setting is provided, use user setting, else
        // use default setting
        if (user_settings.find(key) != user_settings.end()) {
            value_string = user_settings[key];
        } else {
            value_string = info.default_value;
        }

        // Validate and assign value to relevant struct
        validate_and_assign(value_string, info);
    }

    // Check for invalid keys provided by user
    for (auto setting : user_settings) {
        if (settings_map.find(setting.first) == settings_map.end()) {
            throw std::runtime_error("Invalid key " + setting.first + ".\n");
        }
    }
}

void Settings::validate_and_assign(const std::string &value_string, const SettingInfo &info) {
    check_allowed_values(value_string, info.allowed_values);
    if (info.setting_type == INTEGER) {
        int value = std::stoi(value_string);
        *((int *)info.assignment_pointer) = value;
    } else if (info.setting_type == POSITIVE_INTEGER) {
        int value = std::stoi(value_string);
        if (value < 0) {
            throw std::runtime_error("Invalid value " + value_string + ".\n");
        }
        *((int *)info.assignment_pointer) = value;
    } else if (info.setting_type == NEGATIVE_INTEGER) {
        int value = std::stoi(value_string);
        if (value > 0) {
            throw std::runtime_error("Invalid value " + value_string + ".\n");
        }
        *((int *)info.assignment_pointer) = value;
    } else if (info.setting_type == FLOAT) {
        double value = std::stof(value_string);
        *((double *)info.assignment_pointer) = value;
    } else if (info.setting_type == POSITIVE_FLOAT) {
        double value = std::stof(value_string);
        if (value < 0) {
            throw std::runtime_error("Invalid value " + value_string + ".\n");
        }
        *((double *)info.assignment_pointer) = value;
    } else if (info.setting_type == NEGATIVE_FLOAT) {
        double value = std::stof(value_string);
        if (value > 0) {
            throw std::runtime_error("Invalid value " + value_string + ".\n");
        }
        *((double *)info.assignment_pointer) = value;
    } else if (info.setting_type == TEXT) {
        std::string value = value_string;
        *((std::string *)info.assignment_pointer) = value;
    }
}

void Settings::check_allowed_values(const std::string &value_string, const std::set<std::string> &allowed_values) {
    if (!allowed_values.empty()) {
        if (allowed_values.find(value_string) == allowed_values.end()) {
            throw std::runtime_error("Invalid value " + value_string + ".\n");
        }
    }
}