/**
 * @file shaderLoader.hpp
 * @author Christian Galvez
 * @brief Loads fragment and vertex shaders into application context
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once
#include <string>
#include <common.hpp>
GLuint loadShaders(string vertexShader, string fragmentShader);
