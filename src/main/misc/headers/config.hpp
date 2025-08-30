/**
 * @file config.hpp
 * @author Christian Galvez, Alec Jackson
 * @brief Contains structs and function prototypes for config.cpp
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include <string>
#include <map>
#include <common.hpp>
#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 720
#define DEFAULT_VSYNC 1
#define DEFAULT_GFX "OpenGL"

enum class ConfigStatus {
  SUCCESS,
  FAILURE
};

template<typename T>
class ConfigResult {
 public:
  T data;
  ConfigStatus status;
  inline bool success() const { return status == ConfigStatus::SUCCESS; }
  inline explicit ConfigResult(T d, ConfigStatus s) :
    data { d }, status { s } {}
};

class StudiousConfig {
 public:
  explicit StudiousConfig(string configPath);
  ConfigResult<int> getIField(string fieldName) const;
  ConfigResult<uint> getUField(string fieldName) const;
  ConfigResult<string> getSField(string fieldName) const;
 private:
  std::map<string, string> configMap_;
};

