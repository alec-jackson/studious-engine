#pragma once
#include "common.h"
typedef struct polygon {
    GLuint *shapebufferID; // used for vertex buffer
    GLuint *textureCoordsID; // used for texture coordinate buffer
    GLuint *textureID; // ID for texture binding
    GLuint *normalbufferID;
    GLuint textureUniformID; // ID for finding texture sampler in OpenGL table
    GLfloat x,y,z;
    GLfloat **vertices; // RAW vertex data
    GLfloat **textureCoords; // RAW texture coordinate data
    GLfloat **normalCoords; // RAW normal vector data
    GLint *pointCount; // describes the number of distinct points in each array
    GLint numberOfObjects; // Contains the number of objects in the model
    GLuint programID; // Used for storing most recent shader
} polygon;

/*
The importObjInfo struct is used as an argument for the importObj function. All
of the current arguments for importObj will be morphed into this single struct
with the same names.
*/
typedef struct importObjInfo {
	const char *modelPath;
	vector<string> texturePath;
	int numTextures, *texturePattern;
	GLuint programID;
} importObjInfo;

polygon *importObj(importObjInfo objInfo);
