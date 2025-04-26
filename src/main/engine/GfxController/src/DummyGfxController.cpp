/**
 * @file DummyGfxController.cpp
 * @author Christian Galvez
 * @brief 
 * @version 0.1
 * @date 2024-01-21
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <string>
#include <iostream>
#include <vector>
#include <cstdio>
#include <DummyGfxController.hpp>

GfxResult<unsigned int> DummyGfxController::generateBuffer(unsigned int *bufferId) {
    printf("GfxController::generateBuffer %p\n", bufferId);
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> DummyGfxController::generateTexture(unsigned int *textureId) {
    printf("GfxController::generateTexture %p\n", textureId);
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> DummyGfxController::bindBuffer(unsigned int bufferId) {
    printf("GfxController::bindBuffer %d\n", bufferId);
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> DummyGfxController::sendBufferData(size_t size, void *data) {
    printf("GfxController::sendBufferData: size %zu, data %p\n", size, data);
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> DummyGfxController::sendTextureData(unsigned int width, unsigned int height, TexFormat format,
    void *data) {
    printf("GfxController::sendTextureData: width %u, height %u, format %d, data %p\n",
        width, height, static_cast<std::underlying_type_t<TexFormat>>(format), data);
    return GFX_OK(unsigned int);
}

GfxResult<int> DummyGfxController::getShaderVariable(unsigned int, const char *) {
    cout << "GfxController::getShaderVariable" << endl;
    return GFX_OK(int);
}

GfxResult<int> DummyGfxController::cleanup() {
    cout << "GfxController::cleanup" << endl;
    return GFX_OK(int);
}

GfxResult<unsigned int> DummyGfxController::getProgramId(uint index) {
    cout << "GfxController::getProgramId" << endl;
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> DummyGfxController::loadShaders(string vertexPath, string fragmentPath) {
    cout << "GfxController::loadShaders" << endl;
    return GFX_OK(unsigned int);
}

void DummyGfxController::update() {
    cout << "GfxController::update" << endl;
}

GfxResult<int> DummyGfxController::init() {
    cout << "GfxController::init" << endl;
    return GFX_OK(int);
}

GfxResult<unsigned int> DummyGfxController::setProgram(unsigned int programId) {
    cout << "GfxController::setProgram" << endl;
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> DummyGfxController::sendFloat(unsigned int variableId, float data) {
    printf("GfxController::sendFloat: variableId=[%u], data=[%f]", variableId, data);
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> DummyGfxController::sendFloatVector(unsigned int variableId, size_t count, float *data) {
    printf("GfxController::sendFloatVector: variableId=[%u], count=[%zu], data=[%p]\n",
        variableId,
        count,
        data);
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> DummyGfxController::polygonRenderMode(RenderMode mode) {
    printf("GfxController::polygonRenderMode: mode=[%d]", static_cast<std::underlying_type_t<RenderMode>>(mode));
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> DummyGfxController::sendFloatMatrix(unsigned int variableId, size_t count, float *data) {
    printf("GfxController::sendFloatMatrix: variableId=[%u], count=[%zu], data=[%p]\n",
        variableId,
        count,
        data);
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> DummyGfxController::sendInteger(unsigned int variableId, int data) {
    printf("GfxController::sendInteger: variableId=[%u], data=[%d]", variableId, data);
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> DummyGfxController::bindTexture(unsigned int textureId) {
    printf("GfxController::bindTexture: textureId=[%u]", textureId);
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> DummyGfxController::bindVao(unsigned int vao) {
    printf("GfxController::bindVao: VAO ID %d\n", vao);
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> DummyGfxController::setCapability(GfxCapability capabilityId, bool enabled) {
    printf("GfxController::setCapability: CAPABILITY: %d, enabled: %d\n",
        static_cast<std::underlying_type_t<GfxCapability>>(capabilityId), enabled);
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> DummyGfxController::initVao(unsigned int *vao) {
    printf("GfxController::initVao: vao %p\n", vao);
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> DummyGfxController::deleteTextures(unsigned int *tId) {
    printf("GfxController::initVao: tId %p\n", tId);
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> DummyGfxController::updateBufferData(const vector<float> &vertices, unsigned int vbo) {
    printf("GfxController::updateBufferData\n");
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> DummyGfxController::setTexParam(TexParam param, TexVal val) {
    printf("GfxController::setTexParam: param %d, val %d\n",
            static_cast<std::underlying_type_t<TexParam>>(param),
            static_cast<std::underlying_type_t<TexValType>>(val.type()));
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> DummyGfxController::generateMipMap() {
    printf("GfxController::generateMipMap\n");
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> DummyGfxController::enableVertexAttArray(unsigned int layout, size_t size) {
    printf("GfxController::enableVertexAttArray: layout %d, size %zu\n",
        layout, size);
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> DummyGfxController::disableVertexAttArray(unsigned int layout) {
    printf("GfxController::disableVertexAttArray: layout %d\n",
        layout);
    return GFX_OK(unsigned int);
}

GfxResult<unsigned int> DummyGfxController::drawTriangles(unsigned int size) {
    printf("GfxController::drawTriangles: size %d\n",
        size);
    return GFX_OK(unsigned int);
}

void DummyGfxController::clear(GfxClearMode clearMode) {
    printf("GfxController::clear: clearMode %d\n",
        static_cast<std::underlying_type_t<GfxClearMode>>(clearMode));
}

void DummyGfxController::deleteBuffer(unsigned int *bufferId) {
    printf("GfxController::deleteBuffer: %p\n",
        bufferId);
}

void DummyGfxController::deleteVao(unsigned int *vao) {
    printf("GfxController::deleteBuffer: %p\n",
        vao);
}

void DummyGfxController::setBgColor(float r, float g, float b) {
    printf("GfxController::setBgColor: %f, %f, %f\n",
        r, g, b);
}

