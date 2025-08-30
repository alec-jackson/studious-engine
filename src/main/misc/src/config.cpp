/**
 * @file config.cpp
 * @author Alec Jackson, Christian Galvez
 * @brief Configures runtime application settings
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <string>
#include <cstdio>
#include <iostream>
#include <config.hpp>

StudiousConfig::StudiousConfig(string configPath) {
    ifstream file;
    file.open(configPath);
    if (!file.is_open()) {
        fprintf(stderr, "loadConfig: Failed to open config file at %s\n", configPath.c_str());
        return;
    }
    string line;
    while (getline(file, line)) {
        // Parse the line
        auto split = line.find_first_of('=');
        if (split == std::string::npos) {
            printf("loadConfig: Bad syntax on config field %s\n", line.c_str());
            continue;
        }
        auto configField = line.substr(0, split);
        const auto value = line.substr(split + 1, line.length() - split);
        printf("loadConfig: %s = %s\n", configField.c_str(), value.c_str());
        configMap_[configField] = std::string(value.c_str());
    }
}

ConfigResult<int> StudiousConfig::getIField(string fieldName) const {
    // Check if the key is present
    ConfigResult result = ConfigResult(0, ConfigStatus::FAILURE);
    auto cit = configMap_.find(fieldName);
    if (cit != configMap_.end()) {
        result = ConfigResult(std::atoi(cit->second.c_str()), ConfigStatus::SUCCESS);
    }
    return result;
}

// Just returns the result as a uint to avoid the need to re-cast
ConfigResult<uint> StudiousConfig::getUField(string fieldName) const {
    const auto query = getIField(fieldName);
    ConfigResult result = ConfigResult(0u, ConfigStatus::FAILURE);
    if (ConfigStatus::SUCCESS == result.status) {
        result = ConfigResult(static_cast<uint>(query.data), query.status);
    }
    return result;
}

ConfigResult<string> StudiousConfig::getSField(string fieldName) const {
    // Check if the key is present
    ConfigResult result = ConfigResult<string>("", ConfigStatus::FAILURE);
    auto cit = configMap_.find(fieldName);
    if (cit != configMap_.end()) {
        result = ConfigResult<string>(cit->second, ConfigStatus::SUCCESS);
    }
    return result;
}

