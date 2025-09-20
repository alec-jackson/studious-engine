/**
 * @file Model.hpp
 * @author Christian Galvez
 * @brief Model class that represents a renderable object in a game scene
 * @version 0.1
 * @date 2023-07-29
 *
 * @copyright Copyright (c) 2023
 *
 */
#pragma once
#include <vector>
#include <string>
#include <common.hpp>
#include <winsup.hpp>
#include <Material.hpp>

class Model {
 public:
    inline Model(unsigned int pointCount, vector<float> vertices, vector<float> textures,
        vector<float> normals) : vertices{std::move(vertices)},
        textureCoords { std::move(textures) }, normalCoords { std::move(normals) },
        pointCount(pointCount) {}
    inline Model(unsigned int pointCount, vector<float> vertices) :
        vertices { std::move(vertices) }, pointCount(pointCount) {}

    uint shapeBufferId;  // used for vertex buffer
    uint textureCoordsId;  // used for texture coordinate buffer
    uint textureId;  // ID for texture binding
    uint normalBufferId;
    vector<float> vertices;  // 2D vector for vertices
    vector<float> textureCoords;  // 2D vector for texture coord data
    vector<float> normalCoords;  // 2D vector for normal coord data
    uint pointCount;  // no. of distinct points in shape
    string materialName;
};
