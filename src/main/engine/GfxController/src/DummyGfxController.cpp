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
#include <DummyGfxController.hpp>

GfxResult<GLuint> DummyGfxController::generateBuffer(GLuint *bufferId) {
    printf("GfxController::generateBuffer %p\n", bufferId);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::generateTexture(GLuint *textureId) {
    printf("GfxController::generateTexture %p\n", textureId);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::bindBuffer(GLuint bufferId) {
    printf("GfxController::bindBuffer %d\n", bufferId);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::sendBufferData(size_t size, void *data) {
    printf("GfxController::sendBufferData: size %ld, data %p\n", size, data);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::sendTextureData(GLuint width, GLuint height, TexFormat format, void *data) {
    printf("GfxController::sendTextureData: width %u, height %u, format %d, data %p\n",
        width, height, format, data);
    return GFX_OK(GLuint);
}

GfxResult<GLint> DummyGfxController::getShaderVariable(GLuint, const char *) {
    cout << "GfxController::getShaderVariable" << endl;
    return GFX_OK(GLint);
}

GfxResult<GLint> DummyGfxController::cleanup() {
    cout << "GfxController::cleanup" << endl;
    return GFX_OK(GLint);
}

GfxResult<GLuint> DummyGfxController::getProgramId(uint index) {
    cout << "GfxController::getProgramId" << endl;
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::loadShaders(string vertexPath, string fragmentPath) {
    cout << "GfxController::loadShaders" << endl;
    return GFX_OK(GLuint);
}

void DummyGfxController::update() {
    cout << "GfxController::update" << endl;
}

GfxResult<GLint> DummyGfxController::init() {
    cout << "GfxController::init" << endl;
    return GFX_OK(GLint);
}

GfxResult<GLuint> DummyGfxController::setProgram(GLuint programId) {
    cout << "GfxController::setProgram" << endl;
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::sendFloat(GLuint variableId, GLfloat data) {
    printf("GfxController::sendFloat: variableId=[%u], data=[%f]", variableId, data);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::sendFloatVector(GLuint variableId, GLsizei count, GLfloat *data) {
    printf("GfxController::sendFloatVector: variableId=[%u], count=[%d], data=[%p]\n",
        variableId,
        count,
        data);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::polygonRenderMode(RenderMode mode) {
    printf("GfxController::polygonRenderMode: mode=[%d]", mode);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::sendFloatMatrix(GLuint variableId, GLsizei count, GLfloat *data) {
    printf("GfxController::sendFloatMatrix: variableId=[%u], count=[%d], data=[%p]\n",
        variableId,
        count,
        data);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::sendInteger(GLuint variableId, GLint data) {
    printf("GfxController::sendInteger: variableId=[%u], data=[%d]", variableId, data);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::bindTexture(GLuint textureId) {
    printf("GfxController::bindTexture: textureId=[%u]", textureId);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::bindVao(GLuint vao) {
    printf("GfxController::bindVao: VAO ID %d\n", vao);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::setCapability(GfxCapability capabilityId, bool enabled) {
    printf("GfxController::setCapability: CAPABILITY: %d, enabled: %d\n",
        capabilityId, enabled);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::initVao(GLuint *vao) {
    printf("GfxController::initVao: vao %p\n", vao);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::deleteTextures(GLuint *tId) {
    printf("GfxController::initVao: tId %p\n", tId);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::updateBufferData(const vector<GLfloat> &vertices, GLuint vbo) {
    printf("GfxController::updateBufferData\n");
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::setTexParam(TexParam param, TexVal val) {
    printf("GfxController::setTexParam: param %d, val %d\n",
        param, val.type());
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::generateMipMap() {
    printf("GfxController::generateMipMap\n");
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::enableVertexAttArray(GLuint layout, size_t size) {
    printf("GfxController::enableVertexAttArray: layout %d, size %ld\n",
        layout, size);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::disableVertexAttArray(GLuint layout) {
    printf("GfxController::disableVertexAttArray: layout %d\n",
        layout);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::drawTriangles(GLuint size) {
    printf("GfxController::drawTriangles: size %d\n",
        size);
    return GFX_OK(GLuint);
}

