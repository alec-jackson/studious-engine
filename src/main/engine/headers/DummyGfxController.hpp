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
    GfxResult<GLint> getShaderVariable(GLint, const char *) const;
    GfxResult<GLint> cleanup();
    GfxResult<GLuint> getProgramId(uint);
    GfxResult<GLuint> setProgram(GLuint) const;
    GfxResult<GLuint> loadShaders(string, string);
    GfxResult<GLuint> sendFloat(GLuint variableId, GLfloat data) const;
    GfxResult<GLuint> sendFloatVector(GLuint variableId, GLsizei count, GLfloat *data) const;
    GfxResult<GLuint> polygonRenderMode(RenderMode mode) const;
    void update();
};
