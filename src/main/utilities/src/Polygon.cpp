/**
 * @file Polygon.cpp
 * @author Christian Galvez
 * @brief Implementation of Polygon class
 * @version 0.1
 * @date 2023-07-29
 * 
 * @copyright Copyright (c) 2023
 * 
 */

// Include External Libaries
#include <iostream>
#include <utility>
#include <vector>

// Include Internal Headers
#include <Polygon.hpp>

using std::cout;
using std::endl;

static int polyCount;

/**
 * @brief Constructs a new Polygon, adds texture coordinates/normal vectors
 * 
 * @param triCount Number of triangles in the shape
 * @param programId ProgramId used to identify the shader set associated with this polygon
 * @param vertices Vertex points for triangles making up the polygon
 * @param textures Texture coordinates for the polygon
 * @param normals Normal vector for the polygon tri faces
 */
Polygon::Polygon(unsigned int triCount, vector<float> vertices, vector<float> textures,
    vector<float> normals) : Polygon(triCount, vertices) {
    cout << "Polygon::Polygon: More complex constructor: polyCount[" << polyCount << "]"
        << endl;

    /* Add textures*/
    this->textureCoords.push_back(textures);
    this->normalCoords.push_back(normals);
}

/**
 * @brief Constructs a new Polygon
 * 
 * @param pointCount Number of triangles in the shape
 * @param programId ProgramId used to identify the shader set associated with this polygon
 * @param vertices Vertex points for triangles making up the polygon
 */
Polygon::Polygon(unsigned int pointCount, vector<float> vertices) :
    pointCount { pointCount }, numberOfObjects { 1 }, textureUniformId { 0 } {
    cout << "Polygon::Polygon: Basic constructor: polyCount[" << polyCount << "] -> [" << polyCount + 1 << "]" << endl;
    ++polyCount;

    this->vertices.push_back(vertices);

    // Push zeroes into vectors for glBuffer functions to write into
    this->shapeBufferId.push_back(0);
    this->normalBufferId.push_back(0);

    // Texture values will default to UINT_MAX to signify no texture
    this->textureId.push_back(UINT_MAX);
    this->textureCoordsId.push_back(UINT_MAX);
}

Polygon::Polygon() : numberOfObjects { 0 }, textureUniformId { 0 } {
    cout << "Polygon::Polygon: Empty constructor: polyCount[" << polyCount << "] -> [" << polyCount + 1 << "]" << endl;
    ++polyCount;
}

/**
 * @brief Destroys the Polygon object
 * 
 */
Polygon::~Polygon() {
    cout << "Polygon::~Polygon: polyCount[" << polyCount << "] -> [" << polyCount - 1 << "]" << endl;
    --polyCount;
}

void Polygon::merge(const Polygon &polygon) {
    cout << "Polygon::merge: Merging polygons!" << endl;
    // Copy over VBO objects from other polygon
    this->vertices.insert(this->vertices.end(), polygon.vertices.begin(), polygon.vertices.end());
    this->textureCoords.insert(this->textureCoords.end(), polygon.textureCoords.begin(), polygon.textureCoords.end());
    this->normalCoords.insert(this->normalCoords.end(), polygon.normalCoords.begin(), polygon.normalCoords.end());

    // Copy over Ids from other polygon
    this->shapeBufferId.insert(this->shapeBufferId.end(), polygon.shapeBufferId.begin(), polygon.shapeBufferId.end());
    this->normalBufferId.insert(this->normalBufferId.end(), polygon.normalBufferId.begin(),
        polygon.normalBufferId.end());
    this->textureId.insert(this->textureId.end(), polygon.textureId.begin(), polygon.textureId.end());
    this->textureCoordsId.insert(this->textureCoordsId.end(), polygon.textureCoordsId.begin(),
        polygon.textureCoordsId.end());

    this->pointCount.insert(this->pointCount.end(), polygon.pointCount.begin(), polygon.pointCount.end());

    this->numberOfObjects++;
}

Polygon::Polygon(Polygon&& other) {
    cout << "Polygon::Polygon: Move constructor called: polyCount[" <<
        polyCount << "] -> [" << polyCount + 1 << "]" << endl;
    polyCount++;
    this->shapeBufferId = std::move(other.shapeBufferId);
    this->textureCoordsId = std::move(other.textureCoordsId);
    this->textureId = std::move(other.textureId);
    this->normalBufferId = std::move(other.normalBufferId);
    this->vertices = std::move(other.vertices);
    this->textureCoords = std::move(other.textureCoords);
    this->normalCoords = std::move(other.normalCoords);
    this->pointCount = std::move(other.pointCount);
    this->numberOfObjects = other.numberOfObjects;
    this->textureUniformId = other.textureUniformId;
    this->texturePath_ = std::move(other.texturePath_);
    this->texturePattern_ = std::move(other.texturePattern_);
}
