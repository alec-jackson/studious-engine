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
#include <common.hpp>

/*
 configData contains data parsed from a configuration file for the purposes of
 creating an new SDL window instance. The following struct members are described
 below:
 * (int) resX - Width of the SDL window in pixels.
 * (int) resY - Height of the SDL window in pixels.
*/
typedef struct configData {
  int resX;
  int resY;
  bool enableVsync;
} configData;

int loadConfig(configData* config, string filename);
