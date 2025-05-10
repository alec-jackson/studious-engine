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
    GfxResult<unsigned int> generateBuffer(unsigned int *bufferId);
    GfxResult<unsigned int> generateTexture(unsigned int *textureId);
    GfxResult<unsigned int> bindBuffer(unsigned int bufferId);
    GfxResult<unsigned int> sendBufferData(size_t size, void *data);
    GfxResult<unsigned int> sendTextureData(unsigned int width, unsigned int height, TexFormat format, void *data);
    GfxResult<int> getShaderVariable(unsigned int, const char *);
    GfxResult<int> cleanup();
    GfxResult<unsigned int> getProgramId(uint);
    GfxResult<unsigned int> setProgram(unsigned int programId);
    GfxResult<unsigned int> loadShaders(string, string);
    GfxResult<unsigned int> sendFloat(unsigned int variableId, float data);
    GfxResult<unsigned int> sendFloatVector(unsigned int variableId, size_t count, float *data);
    GfxResult<unsigned int> polygonRenderMode(RenderMode mode);
    GfxResult<unsigned int> sendFloatMatrix(unsigned int variableId, size_t count, float *data);
    GfxResult<unsigned int> sendInteger(unsigned int variableId, int data);
    GfxResult<unsigned int> bindTexture(unsigned int textureId);
    GfxResult<unsigned int> initVao(unsigned int *vao);
    GfxResult<unsigned int> bindVao(unsigned int vao);
    GfxResult<unsigned int> setCapability(GfxCapability capabilityId, bool enabled);
    GfxResult<unsigned int> deleteTextures(unsigned int *tId);
    GfxResult<unsigned int> updateBufferData(const vector<float> &vertices, unsigned int vbo);
    GfxResult<unsigned int> setTexParam(TexParam param, TexVal val);
    GfxResult<unsigned int> generateMipMap();
    GfxResult<unsigned int> enableVertexAttArray(unsigned int layout, size_t size);
    GfxResult<unsigned int> disableVertexAttArray(unsigned int layout);
    GfxResult<unsigned int> drawTriangles(unsigned int size);
    void setBgColor(float r, float g, float b);
    void deleteVao(unsigned int *vao);
    void deleteBuffer(unsigned int *bufferId);
    void clear(GfxClearMode clearMode);
    void update();
    void updateOpenGl();

 private:
    vector<unsigned int> programIdList_;

    /* Objects tracked internally to free when closing */
    vector<unsigned int> vaoList_;
    vector<unsigned int> vboList_;
    vector<unsigned int> textureIdList_;
    vector<float> bgColor_;
};
