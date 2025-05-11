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
#include <winsup.hpp>

#define GFX_OK(gType) GfxResult<gType>(GfxApiResult::OK, 0)
#define GFX_FAILURE(gType) GfxResult<gType>(GfxApiResult::FAILURE, -1)

enum class GfxApiResult {
    OK,
    FAILURE
};

enum class RenderMode {
    POINT,
    LINE,
    FILL
};

enum class TexFormat {
    RGBA,
    RGB,
    BITMAP
};

enum class TexParam {
    WRAP_MODE_S,
    WRAP_MODE_T,
    MINIFICATION_FILTER,
    MAGNIFICATION_FILTER,
    MIPMAP_LEVEL
};

enum class TexValType {
    CLAMP_TO_EDGE,
    GFX_LINEAR,
    NEAREST_MIPMAP,
    NEAREST_NEIGHBOR,
    CUSTOM
};

class TexVal {
 public:
    explicit inline TexVal(TexValType type) : type_ { type } {}
    explicit inline TexVal(int data) : type_ { TexValType::CUSTOM }, data_ { data } {}
    inline TexValType type() { return type_; }
    inline int data() { return data_; }
 private:
    TexValType type_;
    int data_;
};

enum class GfxCapability {
    CULL_FACE
};

enum class GfxClearMode {
    DEPTH,
    COLOR
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
    virtual GfxResult<int> init() = 0;
    virtual GfxResult<unsigned int> generateBuffer(unsigned int *bufferId) = 0;
    virtual GfxResult<unsigned int> generateTexture(unsigned int *textureId) = 0;
    virtual GfxResult<unsigned int> bindBuffer(unsigned int bufferId) = 0;
    virtual GfxResult<unsigned int> sendBufferData(size_t size, void *data) = 0;
    virtual GfxResult<unsigned int> sendTextureData(unsigned int width, unsigned int height, TexFormat format,
        void *data) = 0;
    virtual GfxResult<int>  getShaderVariable(unsigned int, const char *) = 0;
    virtual GfxResult<unsigned int> getProgramId(uint) = 0;
    virtual GfxResult<unsigned int> setProgram(unsigned int) = 0;
    virtual GfxResult<unsigned int> loadShaders(string, string) = 0;
    virtual GfxResult<unsigned int> sendFloat(unsigned int variableId, float data) = 0;
    virtual GfxResult<unsigned int> sendFloatVector(unsigned int variableId, size_t count, float *data) = 0;
    virtual GfxResult<unsigned int> polygonRenderMode(RenderMode mode) = 0;
    virtual GfxResult<unsigned int> sendFloatMatrix(unsigned int variableId, size_t count, float *data) = 0;
    virtual GfxResult<unsigned int> sendInteger(unsigned int variableId, int data) = 0;
    virtual GfxResult<unsigned int> bindTexture(unsigned int textureId) = 0;
    virtual GfxResult<unsigned int> initVao(unsigned int *vao) = 0;
    virtual GfxResult<unsigned int> bindVao(unsigned int vao) = 0;
    virtual GfxResult<unsigned int> setCapability(GfxCapability capabilityId, bool enabled) = 0;
    virtual GfxResult<unsigned int> deleteTextures(unsigned int *tId) = 0;
    virtual GfxResult<unsigned int> updateBufferData(const vector<float> &vertices, unsigned int vbo) = 0;
    virtual GfxResult<unsigned int> setTexParam(TexParam param, TexVal val) = 0;
    virtual GfxResult<unsigned int> generateMipMap() = 0;
    virtual GfxResult<unsigned int> enableVertexAttArray(unsigned int layout, size_t size) = 0;
    virtual GfxResult<unsigned int> disableVertexAttArray(unsigned int layout) = 0;
    virtual GfxResult<unsigned int> drawTriangles(unsigned int size) = 0;
    /**
     * @brief Sets the background color of the window.
     * @param r Red value from 0.0f to 1.0f.
     * @param g Green value from 0.0f to 1.0f.
     * @param b Blue value from 0.0f to 1.0f.
     */
    virtual void setBgColor(float r, float g, float b) = 0;
    virtual void clear(GfxClearMode clearMode) = 0;
    virtual void update() = 0;
    virtual void deleteBuffer(unsigned int *bufferId) = 0;
    virtual void deleteVao(unsigned int *vao) = 0;
};
