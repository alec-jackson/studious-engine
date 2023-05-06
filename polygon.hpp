#ifndef POLYGON_HPP
#define POLYGON_HPP

#include "modelImport.hpp"

class Polygon {
private:
    gluVec shapebufferID; // used for vertex buffer
    vector<GLuint> textureCoordsID; // used for texture coordinate buffer
    vector<GLuint> textureID; // ID for texture binding
    vector<GLuint> normalbufferID;
    GLuint textureUniformID; // ID for finding texture sampler in OpenGL table
    glfVVec vertices; // 2D vector for vertices
    glfVVec textureCoords; // 2D vector for texture coord data
    glfVVec normalCoords; // 2D vector for normal coord data
    vector<GLint> pointCount; // no. of distinct points in shape
    GLint numberOfObjects; // Contains the number of objects in the model
    GLuint programID; // Used for storing programID of object's shader
public:
    Polygon(GLuint, GLuint, GLuint, glfVec, GLint, GLuint);
    Polygon(ImportObjInfo &objInfo);
    void configureObject(const ConfigureArgs &objInfo);
    GLint getNumberOfObjects()const ;
    GLuint getProgramId() const;
    GLuint *getShapeBufferIdAddr(unsigned int index); // SUS!
    GLuint getShapeBufferId(unsigned int index) const; // SUS!
    GLuint getTextureCoordsId(unsigned int index) const; // SUS!
    GLuint getTextureUniformId() const;
    GLuint getNormalBufferId(unsigned int index) const; // SUS!
    GLuint getTextureId(unsigned int index) const; // SUS!
    GLint getPointCount(unsigned int index) const;
    glfVVec getVertices() const;
    GLfloat *getVerticiesLocation(int x, int y);
    inline Polygon() {};
};

#endif