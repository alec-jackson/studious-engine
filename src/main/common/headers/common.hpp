/**
 * @file common.hpp
 * @author Christian Galvez
 * @brief Contains common definitions and imports used across studious-engine
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once
#define SHOW_FPS 1  // Enables FPS logging to console when = 1, = 0 to disable
#include <cstdio>
#include "SDL.h"
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <cmath>
#include <string>
#include <vector>
#include <thread>  //NOLINT
#include <mutex>  //NOLINT
#include <fstream>
#include <map>
#include <chrono> //NOLINT
#ifndef _WIN32  // Don't use unistd.h on windows
#include <unistd.h>
#else
#include <Windows.h>
#define usleep(x) Sleep(x / 1000)  // Windows does not have these nice sleep functions :(
#define sleep(x) Sleep(x * 1000)
#endif
#include <iostream>
#define SDL2_image  // ? Should we remove this?
#define GLM_ENABLE_EXPERIMENTAL  // Keep this, I don't really remember why but just do it
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using glm::ivec2;
using glm::vec3;
using glm::vec4;

using glm::mat4;

using glm::ortho;
using glm::perspective;
using glm::radians;

using std::cout;
using std::cerr;
using std::endl;

using std::vector;
using std::string;
using std::map;
using std::to_string;

using std::mutex;
using std::thread;
using std::ifstream;

using std::runtime_error;

/// @todo - move away from common.hpp anti-pattern
enum ViewMode { ORTHOGRAPHIC, PERSPECTIVE };
