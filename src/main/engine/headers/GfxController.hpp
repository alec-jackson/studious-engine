/**
 * @file GfxController.hpp
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
#include <vector>
#include <Polygon.hpp>

#define GFX_OK(gType) GfxResult<gType>(GfxApiResult::OK, 0);
#define GFX_FAILURE(gType) GfxResult<gType>(GfxApiResult::FAILURE, -1);

enum GfxApiResult {
    OK,
    FAILURE
};

enum RenderMode {
    POINT,
    LINE,
    FILL
};

template <typename T>
class GfxResult {
 public:
    inline GfxResult(GfxApiResult result, T data) : result_ { result }, data_ { data } {}
    inline bool isOk() { result_ == GfxApiResult::OK; }
    inline GfxApiResult result() { return result_; }
    inline T get() { return data_; }
 private:
    GfxApiResult result_;
    T data_;
};

class GfxController {
 public:
    virtual GfxResult<GLint> init() = 0;
    virtual GfxResult<GLint> generateVertexBuffer(Polygon &) = 0;
    virtual GfxResult<GLint> generateNormalBuffer(Polygon &) = 0;
    virtual GfxResult<GLint> generateTextureBuffer(Polygon &, SDL_Surface *) = 0;
    virtual GfxResult<GLint> getShaderVariable(GLint, const char *) const = 0;
    virtual GfxResult<GLint> cleanup() = 0;
    virtual GfxResult<GLuint> getProgramId(uint) = 0;
    virtual GfxResult<GLuint> setProgram(GLuint) const = 0;
    virtual GfxResult<GLuint> loadShaders(string, string) = 0;
    virtual GfxResult<GLuint> sendFloat(GLuint variableId, GLfloat data) const = 0;
    virtual GfxResult<GLuint> sendFloatVector(GLuint variableId, GLsizei count, GLfloat *data) const = 0;
    virtual GfxResult<GLuint> polygonRenderMode(RenderMode mode) const = 0;
    virtual void update() = 0;
};
