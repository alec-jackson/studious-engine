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
#include <GfxController.hpp>
#include <Polygon.hpp>
#include <common.hpp>

class OpenGlGfxController : public GfxController {
 public:
    GfxResult<GLint> generateVertexBuffer(Polygon &);
    GfxResult<GLint> generateNormalBuffer(Polygon &);
    GfxResult<GLint> generateTextureBuffer(Polygon &, SDL_Surface *);
    GfxResult<GLint> getShaderVariable(GLint, const char *) const;
    GfxResult<GLint> cleanupPrograms();
    GfxResult<GLuint> getProgramId(uint);
    GfxResult<GLuint> loadShaders(string, string);
 private:
   vector<GLuint> programIdList_;
};
