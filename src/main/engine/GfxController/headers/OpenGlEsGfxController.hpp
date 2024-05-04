/**
 * @file OpenGlEsGfxController.hpp
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
#include <map>
#include <GfxController.hpp>
#include <Polygon.hpp>
#include <common.hpp>
// Temporary until we get a logger, disables noisy OpenGL logs
#define VERBOSE_LOGS

struct GfxVaoData {
   GLuint layout;
   size_t size;
};

class OpenGlEsGfxController : public GfxController {
 public:
    GfxResult<GLint> init();
    GfxResult<GLuint> generateBuffer(GLuint *bufferId);
    GfxResult<GLuint> generateTexture(GLuint *textureId);
    GfxResult<GLuint> bindBuffer(GLuint bufferId);
    GfxResult<GLuint> sendBufferData(size_t size, void *data);
    GfxResult<GLuint> sendTextureData(GLuint width, GLuint height, TexFormat format, void *data);
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
    GfxResult<GLuint> bindTexture(GLuint textureId);
    GfxResult<GLuint> initVao(GLuint *vao);
    GfxResult<GLuint> bindVao(GLuint vao);
    GfxResult<GLuint> setCapability(GfxCapability capabilityId, bool enabled);
    GfxResult<GLuint> deleteTextures(GLuint *tId);
    GfxResult<GLuint> updateBufferData(const vector<GLfloat> &vertices, GLuint vbo);
    GfxResult<GLuint> setTexParam(TexParam param, TexVal val);
    GfxResult<GLuint> generateMipMap();
    GfxResult<GLuint> enableVertexAttArray(GLuint layout, size_t size);
    GfxResult<GLuint> disableVertexAttArray(GLuint layout);
    GfxResult<GLuint> drawTriangles(GLuint size);
    void deleteVao(GLuint *vao);
    void deleteBuffer(GLuint *bufferId);
    void clear(GfxClearMode clearMode);
    void update();
    void updateOpenGl();

 private:
    vector<GLuint> programIdList_;
    // VAO -> (VBO -> Attribute Data)
    map<GLuint, map<GLuint, GfxVaoData>> vaoBindData_;
    GLuint activeVao_ = 0;
    GLuint activeVbo_ = 0;
};
