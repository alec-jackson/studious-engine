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

enum TexFormat {
    RGBA,
    RGB,
    BITMAP
};

enum TexParam {
    WRAP_MODE_S,
    WRAP_MODE_T,
    MINIFICATION_FILTER,
    MAGNIFICATION_FILTER,
    MIPMAP_LEVEL
};

enum TexValType {
    CLAMP_TO_EDGE,
    GFX_LINEAR,
    NEAREST_MIPMAP,
    NEAREST_NEIGHBOR,
    CUSTOM
};

class TexVal {
 public:
    explicit inline TexVal(TexValType type) : type_ { type } {}
    explicit inline TexVal(GLint data) : type_ { TexValType::CUSTOM }, data_ { data } {}
    inline TexValType type() { return type_; }
    inline GLint data() { return data_; }
 private:
    TexValType type_;
    GLint data_;
};

template <typename T>
class GfxResult {
 public:
    inline GfxResult(GfxApiResult result, T data) : result_ { result }, data_ { data } {}
    inline bool isOk() { return result_ == GfxApiResult::OK; }
    inline GfxApiResult result() { return result_; }
    inline T get() { return data_; }
 private:
    GfxApiResult result_;
    T data_;
};

class GfxController {
 public:
    virtual GfxResult<GLint> init() = 0;
    virtual GfxResult<GLuint> generateBuffer(GLuint *bufferId) = 0;
    virtual GfxResult<GLuint> generateTexture(GLuint *textureId) = 0;
    virtual GfxResult<GLuint> bindBuffer(GLuint bufferId) = 0;
    virtual GfxResult<GLuint> sendBufferData(size_t size, void *data) = 0;
    virtual GfxResult<GLuint> sendTextureData(GLuint width, GLuint height, TexFormat format, void *data) = 0;
    virtual GfxResult<GLint>  getShaderVariable(GLuint, const char *) = 0;
    virtual GfxResult<GLint>  cleanup() = 0;
    virtual GfxResult<GLuint> getProgramId(uint) = 0;
    virtual GfxResult<GLuint> setProgram(GLuint) = 0;
    virtual GfxResult<GLuint> loadShaders(string, string) = 0;
    virtual GfxResult<GLuint> sendFloat(GLuint variableId, GLfloat data) = 0;
    virtual GfxResult<GLuint> sendFloatVector(GLuint variableId, GLsizei count, GLfloat *data) = 0;
    virtual GfxResult<GLuint> polygonRenderMode(RenderMode mode) = 0;
    virtual GfxResult<GLuint> sendFloatMatrix(GLuint variableId, GLsizei count, GLfloat *data) = 0;
    virtual GfxResult<GLuint> sendInteger(GLuint variableId, GLint data) = 0;
    virtual GfxResult<GLuint> bindTexture(GLuint textureId, GLuint samplerId) = 0;
    virtual GfxResult<GLuint> initVao(GLuint *vao) = 0;
    virtual GfxResult<GLuint> bindVao(GLuint vao) = 0;
    virtual GfxResult<GLuint> setCapability(int capabilityId, bool enabled) = 0;
    virtual GfxResult<GLuint> deleteTextures(GLuint *tId) = 0;
    virtual GfxResult<GLuint> updateBufferData(const vector<GLfloat> &vertices, GLuint vbo) = 0;
    virtual GfxResult<GLuint> setTexParam(TexParam param, TexVal val) = 0;
    virtual GfxResult<GLuint> generateMipMap() = 0;
    virtual GfxResult<GLuint> enableVertexAttArray(GLuint layout, size_t size) = 0;
    virtual GfxResult<GLuint> disableVertexAttArray(GLuint layout) = 0;
    virtual GfxResult<GLuint> drawTriangles(GLuint size) = 0;
    virtual void update() = 0;
};
