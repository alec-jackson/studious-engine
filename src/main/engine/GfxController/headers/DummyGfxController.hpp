/**
 * @file DummyGfxController.hpp
 * @author Christian Galvez
 * @brief 
 * @version 0.1
 * @date 2024-01-21
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once
#include <string>
#include <GfxController.hpp>

class DummyGfxController : public GfxController {
 public:
    GfxResult<GLint> init();
    GfxResult<GLint> generateVertexBuffer(Polygon &);
    GfxResult<GLint> generateNormalBuffer(Polygon &);
    GfxResult<GLint> generateTextureBuffer(Polygon &, SDL_Surface *);
    GfxResult<GLuint> generateFontTextures(GLuint width, GLuint rows, unsigned char *buffer);
    GfxResult<GLint> getShaderVariable(GLuint, const char *);
    GfxResult<GLint> cleanup();
    GfxResult<GLuint> getProgramId(uint);
    GfxResult<GLuint> setProgram(GLuint);
    GfxResult<GLuint> loadShaders(string, string);
    GfxResult<GLuint> sendFloat(GLuint variableId, GLfloat data);
    GfxResult<GLuint> sendFloatVector(GLuint variableId, GLsizei count, GLfloat *data);
    GfxResult<GLuint> polygonRenderMode(RenderMode mode);
    GfxResult<GLuint> sendFloatMatrix(GLuint variableId, GLsizei count, GLfloat *data);
    GfxResult<GLuint> sendInteger(GLuint variableId, GLint data);
    GfxResult<GLuint> bindTexture(GLuint textureId, GLuint samplerId);
    GfxResult<GLuint> render(GLuint vao, GLuint vId, GLuint tId, GLuint nId, GLuint vertexCount, GLuint dimension);
    GfxResult<GLuint> initVao(GLuint *vao);
    GfxResult<GLuint> bindVao(GLuint vao);
    GfxResult<GLuint> setCapability(int capabilityId, bool enabled);
    GfxResult<GLuint> deleteTextures(GLuint *tId);
    GfxResult<GLuint> updateBufferData(vector<GLfloat> &vertices, GLuint vbo);
    void update();
};
