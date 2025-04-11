/**
 * @file winsup.hpp
 * @author Christian Galvez
 * @brief Header for helping add proper Windows support across studious.
 * @version 0.1
 * @date 2025-04-11
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#ifdef _WIN32
typedef unsigned int uint;
#define Polygon WIN_Polygon
#endif
