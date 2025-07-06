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

enum class GfxTextureType {
    NORMAL,
    ARRAY
};

enum class VectorType {
    GFX_2D,
    GFX_3D,
    GFX_4D
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

struct ProgramData {
    string programName;
    string vertexShaderPath;
    string fragmentShaderPath;
};

class GfxController {
 public:
    virtual GfxResult<int> init() = 0;
    virtual GfxResult<uint> generateBuffer(uint *bufferId) = 0;
    virtual GfxResult<uint> generateTexture(uint *textureId) = 0;
    virtual GfxResult<uint> bindBuffer(uint bufferId) = 0;
    virtual GfxResult<uint> sendBufferData(size_t size, void *data) = 0;
    virtual GfxResult<uint> sendTextureData(uint width, uint height, TexFormat format,
        void *data) = 0;
    virtual GfxResult<uint> sendTextureData3D(int offsetx, int offsety, int index, uint width, uint height,
        TexFormat format, void *data) = 0;
    virtual GfxResult<int>  getShaderVariable(uint, const char *) = 0;
    /**
     * @brief Fetches the program ID that belongs to the given name. Returns a
     * GFX_FAILURE if the program does not exist, or is inactive.
     */
    virtual GfxResult<uint> getProgramId(string) = 0;
    virtual GfxResult<uint> setProgram(uint) = 0;
    /**
     * @brief Compiles the provided shaders and creates a new program ID on success.
     * @param programName The name to give the newly created program.
     * @param vertShadedPath Path to the vertex shader to compile.
     * @param fragShaderPath Path to the fragment sahder to compile.
     * @return OK on success, FAILURE on failure. When successful, the program ID will be returned in the
     * GfxResult's data field, accessable via GfxResult::get().
     */
    virtual GfxResult<uint> loadShaders(string programName, string vertShaderPath, string fragShaderPath) = 0;
    virtual GfxResult<uint> sendFloat(uint variableId, float data) = 0;
    virtual GfxResult<uint> sendFloatVector(uint variableId, size_t count, VectorType vType, float *data) = 0;
    virtual GfxResult<uint> polygonRenderMode(RenderMode mode) = 0;
    virtual GfxResult<uint> sendFloatMatrix(uint variableId, size_t count, float *data) = 0;
    virtual GfxResult<uint> sendInteger(uint variableId, int data) = 0;
    /**
     * @brief Binds a texture to the current OpenGL context.
     *
     * @param textureId ID of texture to bind.
     * @param type The type of texture being bound.
     * @return GfxResult<unsigned int> OK if successful; FAILURE otherwise
     */
    virtual GfxResult<uint> bindTexture(uint textureId, GfxTextureType type) = 0;
    virtual GfxResult<uint> initVao(uint *vao) = 0;
    virtual GfxResult<uint> bindVao(uint vao) = 0;
    virtual GfxResult<uint> setCapability(GfxCapability capabilityId, bool enabled) = 0;
    virtual GfxResult<uint> deleteTextures(uint *tId) = 0;
    virtual GfxResult<uint> updateBufferData(const vector<float> &vertices, uint vbo) = 0;
    virtual GfxResult<uint> setTexParam(TexParam param, TexVal val, GfxTextureType type) = 0;
    virtual GfxResult<uint> generateMipMap() = 0;
    /**
     * @brief Enables a vertex attribute array and configures the vertex attribute pointer.
     *
     * @param layout The layout set in the OpenGL shader.
     * @param count The number of elements in the attribute array.
     * @param size The size of the underlying type used in bytes. Use sizeof() operator.
     * @param offset Offset of the data in GPU memory. Normally zero unless combining multiple attributes into one.
     * @return GfxResult<uint> OK if succeeded, FAILURE if error occurred.
     */
    virtual GfxResult<uint> enableVertexAttArray(uint layout, int count, size_t size, void *offset) = 0;
    /**
     * @brief Configures the divisor for the attribute in a GLSL shader.
     *
     * @param layout Attribute to target.
     * @param divisor Divisor to use. Use zero for none.
     * @return GfxResult<uint> OK if succeeded, FAILURE if error occurred.
     */
    virtual GfxResult<uint> setVertexAttDivisor(uint layout, uint divisor) = 0;
    virtual GfxResult<uint> disableVertexAttArray(uint layout) = 0;
    virtual GfxResult<uint> drawTriangles(uint size) = 0;
    virtual GfxResult<uint> drawTrianglesInstanced(uint size, uint count) = 0;
    virtual GfxResult<uint> allocateTexture3D(TexFormat format, uint width, uint height, uint layers) = 0;
    /**
     * @brief Sets the background color of the window.
     * @param r Red value from 0.0f to 1.0f.
     * @param g Green value from 0.0f to 1.0f.
     * @param b Blue value from 0.0f to 1.0f.
     */
    virtual void setBgColor(float r, float g, float b) = 0;
    virtual void clear(GfxClearMode clearMode) = 0;
    virtual void update() = 0;
    virtual void deleteBuffer(uint *bufferId) = 0;
    virtual void deleteVao(uint *vao) = 0;
};
