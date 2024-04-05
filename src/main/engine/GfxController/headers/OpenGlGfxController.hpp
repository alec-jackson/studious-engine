/**
 * @file OpenGlGfxController.hpp
 * @author Christian Galvez
 * @brief 
 * @version 0.1
 * @date 2024-01-21
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once
#include <vector>
#include <string>
#include <GfxController.hpp>
#include <Polygon.hpp>
#include <common.hpp>

class OpenGlGfxController : public GfxController {
 public:
    GfxResult<GLint> init();
    GfxResult<GLuint> generateBuffer(GLuint *bufferId);
    GfxResult<GLuint> generateTexture(GLuint *textureId);
    GfxResult<GLuint> bindBuffer(GLuint bufferId);
    GfxResult<GLuint> sendBufferData(size_t size, void *data);
    GfxResult<GLuint> sendTextureData(GLuint width, GLuint height, bool alpha, void *data);
    GfxResult<GLuint> generateFontTextures(GLuint width, GLuint rows, unsigned char *buffer);
    GfxResult<GLint> getShaderVariable(GLuint, const char *);
    GfxResult<GLint> cleanup();
    GfxResult<GLuint> getProgramId(uint);
    GfxResult<GLuint> setProgram(GLuint programId);
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
    GfxResult<GLuint> setTexParam(TexParam param, TexVal val);
    GfxResult<GLuint> generateMipMap();
    void update();
    void updateOpenGl();

 private:
    vector<GLuint> programIdList_;
};
