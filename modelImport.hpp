#ifndef MODEL_IMPORT_HPP
#define MODEL_IMPORT_HPP
#include "common.hpp"

/*
The importObjInfo struct is used as an argument for the importObj function. All
of the current arguments for importObj will be morphed into this single struct
with the same names.
*/
typedef struct ImportObjInfo {
	string modelPath;
	vector<string> texturePath;
    vector<GLint> texturePattern;
	GLuint programID;
} ImportObjInfo;

/*
The configureArgs struct is strictly used for passing arguments into the
configureObject function inside of modelImport.cpp.
*/
typedef struct ConfigureArgs {
    vector<GLfloat> vertexFrame;
    vector<GLfloat> textureFrame;
    vector<GLfloat> normalFrame;
    vector<GLint> commands;
    GLint index;
    unsigned long textureCount;
    vector<GLint> texturePattern;
    vector<string> texturePath;
} ConfigureArgs;

#endif
