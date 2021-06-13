#pragma once
#include "common.hpp"
/*
The polygon struct holds all of the relevant information for a given shape. The
struct members are defined as the following:
GLuint *shapebufferID - Contains ID of buffer in OpenGL for vertices
GLuint *textureCoordsID - Contains ID of buffer in OpenGL for UV coordinates
GLuint *textureID - Contains ID of texture binding
GLuint *normalbufferID - Contains ID of buffer in OpenGL for normals
GLuint textureUniformID - ID for finding texture sampler in OpenGL table
GLfloat **vertices - RAW vertex data for model (format [x, y, z])
GLfloat **textureCoords - RAW texture coordinate data (format [x, y])
GLfloat **normalCoords - RAW normal vector data (format [x, y, z])
GLint *pointCount - Number of faces for a given model
GLint numberOfObjects - Number of distinct objects in model
GLuint programID - Stores programID used for object

Notes:
* Polygon should not be created manually, should instead be created using
some kind of function like importObj.
* vertices, textureCoords and normalCoords are all 2D dimensional arrays because
a single polygon instance can contain multiple "shapes" read from a single file.
This is because some 3D models contain multiple objects (a hat on a character,
for example). The numberOfObjects variable describes the actual number of shapes
or objects connected to the current polygon.
* When drawing vertices for polygon, pointCount[curr] * 3 should be used, since
models currently must have triangulated faces, where curr is the index of the
object to be drawn.
*/
typedef struct polygon {
    vector<GLuint> shapebufferID; // used for vertex buffer
    vector<GLuint> textureCoordsID; // used for texture coordinate buffer
    vector<GLuint> textureID; // ID for texture binding
    vector<GLuint> normalbufferID;
    GLuint textureUniformID; // ID for finding texture sampler in OpenGL table
    vector<vector<GLfloat>> vertices; // 2D vector for vertices
    vector<vector<GLfloat>> textureCoords; // 2D vector for texture coord data
    vector<vector<GLfloat>> normalCoords; // 2D vector for normal coord data
    vector<GLint> pointCount; // no. of distinct points in shape
    GLint numberOfObjects; // Contains the number of objects in the model
    GLuint programID; // Used for storing programID of object's shader
} polygon;

/*
The importObjInfo struct is used as an argument for the importObj function. All
of the current arguments for importObj will be morphed into this single struct
with the same names.
*/
typedef struct importObjInfo {
	string modelPath;
	vector<string> texturePath;
	int numTextures;
    vector<GLint> texturePattern;
	GLuint programID;
} importObjInfo;

/*
The configureArgs struct is strictly used for passing arguments into the
configureObject function inside of modelImport.cpp.
*/
typedef struct configureArgs {
    vector<GLfloat> vertexFrame;
    vector<GLfloat> textureFrame;
    vector<GLfloat> normalFrame;
    vector<GLint> commands;
    GLint index;
    polygon *model;
    int textureCount;
    vector<GLint> texturePattern;
    vector<string> texturePath;
} configureArgs;

polygon *importObj(importObjInfo objInfo);
