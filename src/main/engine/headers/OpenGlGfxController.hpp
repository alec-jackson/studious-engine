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
    GfxResult<GLint> getShaderVariable(GLuint, const char *) const;
    GfxResult<GLint> cleanup();
    GfxResult<GLuint> getProgramId(uint);
    GfxResult<GLuint> setProgram(GLuint programId) const;
    GfxResult<GLuint> loadShaders(string, string);
    GfxResult<GLuint> sendFloat(GLuint variableId, GLfloat data) const;
    GfxResult<GLuint> sendFloatVector(GLuint variableId, GLsizei count, GLfloat *data) const;
    GfxResult<GLuint> polygonRenderMode(RenderMode mode) const;
    GfxResult<GLuint> sendFloatMatrix(GLuint variableId, GLsizei count, GLfloat *data) const;
    GfxResult<GLuint> sendInteger(GLuint variableId, GLint data) const;
    GfxResult<GLuint> bindTexture(GLuint textureId, GLuint samplerId) const;
    GfxResult<GLuint> render(GLuint vId, GLuint tId, GLuint nId, GLuint vertexCount) const;
    void update();
    void updateOpenGl();
 private:
    vector<GLuint> programIdList_;
    GLuint vertexArrayId_;
};
