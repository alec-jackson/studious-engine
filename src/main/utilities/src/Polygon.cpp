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
Polygon::Polygon(GLint triCount, GLuint programId, vector<GLfloat> vertices, vector<GLfloat> textures,
    vector<GLfloat> normals) : Polygon(triCount, programId, vertices) {
    cout << "Polygon::Polygon: More complex constructor: polyCount[" << polyCount << "] -> [" << ++polyCount << "]"
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
Polygon::Polygon(GLint pointCount, GLuint programId, vector<GLfloat> vertices) : pointCount { pointCount },
    numberOfObjects { 1 }, textureUniformId { 0 }, programId { programId } {
    cout << "Polygon::Polygon: Basic constructor: polyCount[" << polyCount << "] -> [" << ++polyCount << "]" << endl;
    this->vertices.push_back(vertices);

    // Push zeroes into vectors for glBuffer functions to write into
    this->shapeBufferId.push_back(0);
    this->normalBufferId.push_back(0);

    // Texture values will default to UINT_MAX to signify no texture
    this->textureId.push_back(UINT_MAX);
    this->textureCoordsId.push_back(UINT_MAX);
}

Polygon::Polygon() : numberOfObjects { 0 }, textureUniformId { 0 } {
    cout << "Polygon::Polygon: Empty constructor: polyCount[" << polyCount << "] -> [" << ++polyCount << "]" << endl;
}

/**
 * @brief Destroys the Polygon object
 * 
 */
Polygon::~Polygon() {
    cout << "Polygon::~Polygon: polyCount[" << polyCount << "] -> [" << --polyCount << "]" << endl;
}

void Polygon::merge(Polygon &polygon) {
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