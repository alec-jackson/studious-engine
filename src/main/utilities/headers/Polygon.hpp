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
#include <common.hpp>
#include <winsup.hpp>

class Polygon {
 public:
    Polygon(unsigned int pointCount, vector<float> vertices, vector<float> textures,
        vector<float> normals);
    Polygon(unsigned int pointCount, vector<float> vertices);
    Polygon();
    void merge(const Polygon&);
    ~Polygon();

    vector<unsigned int> shapeBufferId;  // used for vertex buffer
    vector<unsigned int> textureCoordsId;  // used for texture coordinate buffer
    vector<unsigned int> textureId;  // ID for texture binding
    vector<unsigned int> normalBufferId;
    vector<vector<float>> vertices;  // 2D vector for vertices
    vector<vector<float>> textureCoords;  // 2D vector for texture coord data
    vector<vector<float>> normalCoords;  // 2D vector for normal coord data
    vector<unsigned int> pointCount;  // no. of distinct points in shape
    int numberOfObjects;  // Contains the number of objects in the model
    int textureUniformId;  // ID for finding texture sampler in OpenGL table

    // Adding these in Polygon for now until we figure out material rendering
    vector<string> texturePath_;
    vector<int> texturePattern_;
};
