/**
 * @file Polygon.hpp
 * @author Christian Galvez
 * @brief Polygon class that represents a renderable object in a game scene
 * @version 0.1
 * @date 2023-07-29
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once
#include <vector>
#include <string>
#include <Model.hpp>
#include <common.hpp>
#include <winsup.hpp>

class Polygon {
 public:
    map<string, Model> modelMap;
    map<string, Material> materialMap;
};
