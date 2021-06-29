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
#endif
