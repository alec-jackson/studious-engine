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
    GfxResult<GLint> generateVertexBuffer(Polygon &);
    GfxResult<GLint> generateNormalBuffer(Polygon &);
    GfxResult<GLint> generateTextureBuffer(Polygon &, SDL_Surface *);
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
    GfxResult<GLuint> render(GLuint vId, GLuint tId, GLuint nId, GLuint vertexCount);
    GfxResult<GLuint> bindVao(GLuint vao);
    void update();
    void updateOpenGl();
 private:
    vector<GLuint> programIdList_;
    GLuint vertexArrayId_;
};
