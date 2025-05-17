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
#include <GL/glew.h>
#include <vector>
#include <string>
#include <GfxController.hpp>
#include <Polygon.hpp>
#include <common.hpp>
// Temporary until we get a logger, disables noisy OpenGL logs
// #define VERBOSE_LOGS

class OpenGlGfxController : public GfxController {
 public:
    OpenGlGfxController();
    ~OpenGlGfxController();
    GfxResult<int> init();
    GfxResult<uint> generateBuffer(uint *bufferId);
    GfxResult<uint> generateTexture(uint *textureId);
    GfxResult<uint> bindBuffer(uint bufferId);
    GfxResult<uint> sendBufferData(size_t size, void *data);
    GfxResult<uint> sendTextureData(uint width, uint height, TexFormat format, void *data);
    GfxResult<uint> sendTextureData3D(uint offsetx, uint offsety, uint index, uint width, uint height, uint layers, TexFormat format,
      void *data);
    GfxResult<int> getShaderVariable(uint, const char *);
    GfxResult<int> cleanup();
    GfxResult<uint> getProgramId(string);
    GfxResult<uint> setProgram(uint programId);
    GfxResult<uint> loadShaders(string, string, string);
    GfxResult<uint> sendFloat(uint variableId, float data);
    GfxResult<uint> sendFloatVector(uint variableId, size_t count, float *data);
    GfxResult<uint> polygonRenderMode(RenderMode mode);
    GfxResult<uint> sendFloatMatrix(uint variableId, size_t count, float *data);
    GfxResult<uint> sendInteger(uint variableId, int data);
    GfxResult<uint> bindTexture(uint textureId, GfxTextureType type);
    GfxResult<uint> initVao(uint *vao);
    GfxResult<uint> bindVao(uint vao);
    GfxResult<uint> setCapability(GfxCapability capabilityId, bool enabled);
    GfxResult<uint> deleteTextures(uint *tId);
    GfxResult<uint> updateBufferData(const vector<float> &vertices, uint vbo);
    GfxResult<uint> setTexParam(TexParam param, TexVal val, GfxTextureType type);
    GfxResult<uint> generateMipMap();
    GfxResult<uint> enableVertexAttArray(uint layout, int count, size_t size, void *offset);
    GfxResult<uint> setVertexAttDivisor(uint layout, uint divisor);
    GfxResult<uint> disableVertexAttArray(uint layout);
    GfxResult<uint> drawTriangles(uint size);
    GfxResult<uint> drawTrianglesInstanced(uint size, uint count);
    GfxResult<uint> allocateTexture3D(TexFormat format, uint width, uint height, uint layers);
    void setBgColor(float r, float g, float b);
    void deleteVao(uint *vao);
    void deleteBuffer(uint *bufferId);
    void clear(GfxClearMode clearMode);
    void update();
    void updateOpenGl();

 private:
    map<string, uint> programIdMap_;

    /* Objects tracked internally to free when closing */
    vector<uint> vaoList_;
    vector<uint> vboList_;
    vector<uint> textureIdList_;
    vector<float> bgColor_;
};
