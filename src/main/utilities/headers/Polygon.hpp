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
#include <common.hpp>

class Polygon {
 public:
    Polygon(GLint pointCount, GLuint programId, vector<GLfloat> vertices, vector<GLfloat> textures,
        vector<GLfloat> normals);
    Polygon(GLint pointCount, GLuint programId, vector<GLfloat> vertices);
    Polygon();
    void merge(Polygon&);
    ~Polygon();

    vector<GLuint> shapeBufferId;  // used for vertex buffer
    vector<GLuint> textureCoordsId;  // used for texture coordinate buffer
    vector<GLuint> textureId;  // ID for texture binding
    vector<GLuint> normalBufferId;
    vector<vector<GLfloat>> vertices;  // 2D vector for vertices
    vector<vector<GLfloat>> textureCoords;  // 2D vector for texture coord data
    vector<vector<GLfloat>> normalCoords;  // 2D vector for normal coord data
    GLint textureUniformId;  // ID for finding texture sampler in OpenGL table
    vector<GLint> pointCount;  // no. of distinct points in shape
    GLint numberOfObjects;  // Contains the number of objects in the model
    GLuint programId;  // Used for storing programId of object's shader
};
