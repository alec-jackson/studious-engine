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

GfxResult<uint> DummyGfxController::generateBuffer(uint *bufferId) {
    printf("GfxController::generateBuffer %p\n", bufferId);
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::generateTexture(uint *textureId) {
    printf("GfxController::generateTexture %p\n", textureId);
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::bindBuffer(uint bufferId) {
    printf("GfxController::bindBuffer %d\n", bufferId);
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::sendBufferData(size_t size, void *data) {
    printf("GfxController::sendBufferData: size %zu, data %p\n", size, data);
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::sendTextureData(uint width, uint height, TexFormat format,
    void *data) {
    printf("GfxController::sendTextureData: width %u, height %u, format %d, data %p\n",
        width, height, static_cast<std::underlying_type_t<TexFormat>>(format), data);
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::sendTextureData3D(int offsetx, int offsety, int index, uint width, uint height,
    TexFormat format, void *data) {
    printf("GfxController::sendTextureData3D: ox %d, oy %d, index %d, width %u, height %u, format %d, data %p\n",
        offsetx, offsety, index, width, height,
        static_cast<std::underlying_type_t<TexFormat>>(format), data);
    return GFX_OK(uint);
}

GfxResult<int> DummyGfxController::getShaderVariable(uint, const char *) {
    cout << "GfxController::getShaderVariable" << endl;
    return GFX_OK(int);
}

GfxResult<int> DummyGfxController::cleanup() {
    cout << "GfxController::cleanup" << endl;
    return GFX_OK(int);
}

GfxResult<uint> DummyGfxController::getProgramId([[maybe_unused]] string programName) {
    cout << "GfxController::getProgramId" << endl;
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::loadShaders([[maybe_unused]] string programName, [[maybe_unused]] string vertexPath,
    [[maybe_unused]] string fragmentPath) {
    cout << "GfxController::loadShaders" << endl;
    return GFX_OK(uint);
}

void DummyGfxController::update() {
    cout << "GfxController::update" << endl;
}

GfxResult<int> DummyGfxController::init() {
    cout << "GfxController::init" << endl;
    return GFX_OK(int);
}

GfxResult<uint> DummyGfxController::setProgram([[maybe_unused]] uint programId) {
    cout << "GfxController::setProgram" << endl;
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::sendFloat(uint variableId, float data) {
    printf("GfxController::sendFloat: variableId=[%u], data=[%f]", variableId, data);
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::sendFloatVector(uint variableId, size_t count, [[maybe_unused]] VectorType vType,
    float *data) {
    printf("GfxController::sendFloatVector: variableId=[%u], count=[%zu], data=[%p]\n",
        variableId,
        count,
        data);
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::polygonRenderMode(RenderMode mode) {
    printf("GfxController::polygonRenderMode: mode=[%d]", static_cast<std::underlying_type_t<RenderMode>>(mode));
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::sendFloatMatrix(uint variableId, size_t count, float *data) {
    printf("GfxController::sendFloatMatrix: variableId=[%u], count=[%zu], data=[%p]\n",
        variableId,
        count,
        data);
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::sendInteger(uint variableId, int data) {
    printf("GfxController::sendInteger: variableId=[%u], data=[%d]", variableId, data);
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::bindTexture(uint textureId, [[maybe_unused]] GfxTextureType type) {
    printf("GfxController::bindTexture: textureId=[%u]", textureId);
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::bindVao(uint vao) {
    printf("GfxController::bindVao: VAO ID %d\n", vao);
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::setCapability(GfxCapability capabilityId, bool enabled) {
    printf("GfxController::setCapability: CAPABILITY: %d, enabled: %d\n",
        static_cast<std::underlying_type_t<GfxCapability>>(capabilityId), enabled);
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::initVao(uint *vao) {
    printf("GfxController::initVao: vao %p\n", vao);
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::deleteTextures(uint *tId) {
    printf("GfxController::initVao: tId %p\n", tId);
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::updateBufferData([[maybe_unused]] const vector<float> &vertices, [[maybe_unused]] uint vbo) {
    printf("GfxController::updateBufferData\n");
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::setTexParam(TexParam param, TexVal val, [[maybe_unused]] GfxTextureType type) {
    printf("GfxController::setTexParam: param %d, val %d\n",
            static_cast<std::underlying_type_t<TexParam>>(param),
            static_cast<std::underlying_type_t<TexValType>>(val.type()));
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::generateMipMap() {
    printf("GfxController::generateMipMap\n");
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::enableVertexAttArray(uint layout, [[maybe_unused]] int count, size_t size, [[maybe_unused]] void *offset) {
    printf("GfxController::enableVertexAttArray: layout %d, size %zu\n",
        layout, size);
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::setVertexAttDivisor(uint layout, uint divisor) {
    printf("GfxController::setVertexAttDivisor: layout %d, divisor %d\n",
        layout, divisor);
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::disableVertexAttArray(uint layout) {
    printf("GfxController::disableVertexAttArray: layout %d\n",
        layout);
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::drawTriangles(uint size) {
    printf("GfxController::drawTriangles: size %d\n",
        size);
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::drawTrianglesInstanced(uint size, uint count) {
    printf("GfxController::drawTrianglesInstanced: size %d, count %d\n",
        size, count);
    return GFX_OK(uint);
}

GfxResult<uint> DummyGfxController::allocateTexture3D(TexFormat format, uint width, uint height, uint layers) {
    printf("GfxController::allocateTexture3D: format %d, width %u, height %u, layers %u\n",
        static_cast<std::underlying_type_t<TexFormat>>(format), width, height, layers);
    return GFX_OK(uint);
}

void DummyGfxController::clear(GfxClearMode clearMode) {
    printf("GfxController::clear: clearMode %d\n",
        static_cast<std::underlying_type_t<GfxClearMode>>(clearMode));
}

void DummyGfxController::deleteBuffer(uint *bufferId) {
    printf("GfxController::deleteBuffer: %p\n",
        bufferId);
}

void DummyGfxController::deleteVao(uint *vao) {
    printf("GfxController::deleteBuffer: %p\n",
        vao);
}

void DummyGfxController::setBgColor(float r, float g, float b) {
    printf("GfxController::setBgColor: %f, %f, %f\n",
        r, g, b);
}
