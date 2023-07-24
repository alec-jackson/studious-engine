#ifndef COMMON_H
#define COMMON_H
#define SHOW_FPS 1 // Enables FPS logging to console when = 1, = 0 to disable
#include <cstdio>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <math.h>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <fstream>
#include <map>
#ifndef _WIN32 // Don't use unistd.h on windows
#include <unistd.h>
#else
#include <Windows.h>
#define usleep(x) Sleep(x / 1000) // Windows does not have these nice sleep functions :(
#define sleep(x) Sleep(x * 1000)
#endif
#include <iostream>
#define SDL2_image // ? Should we remove this?
#include <GL/glew.h>
#define GLM_ENABLE_EXPERIMENTAL // Keep this, I don't really remember why but just do it
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;
using namespace std;

// TODO - move away from common.hpp anti-pattern
enum ViewMode { ORTHOGRAPHIC, PERSPECTIVE };

// TODO: Doxygenize, maybe move somewhere else
bool min_func(float a, float b) {
    return a < b;
}

// TODO: Doxygenize
bool max_func(float a, float b) {
    return a > b;
}

#endif
