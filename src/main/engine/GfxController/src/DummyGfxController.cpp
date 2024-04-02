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

GfxResult<GLint> DummyGfxController::generateVertexBuffer(Polygon &polygon) {
    cout << "GfxController::generateVertexBuffer" << endl;
    return GFX_OK(GLint);
}

GfxResult<GLint> DummyGfxController::generateNormalBuffer(Polygon &polygon) {
    cout << "GfxController::generateNormalBuffer" << endl;
    return GFX_OK(GLint);
}

GfxResult<GLint> DummyGfxController::generateTextureBuffer(Polygon &polygon, SDL_Surface *texture) {
    cout << "GfxController::generateTextureBuffer" << endl;
    return GFX_OK(GLint);
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

GfxResult<GLuint> DummyGfxController::bindTexture(GLuint textureId, GLuint samplerId) {
    printf("GfxController::bindTexture: textureId=[%u], samplerId=[%u]", textureId, samplerId);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::render(GLuint vao, GLuint vId, GLuint tId, GLuint nId, GLuint vertexCount) {
    printf("GfxController::bindTexture: vao=[%u], vId=[%u], tId=[%u], nId=[%u], vertexCount=[%u]",
        vao, vId, tId, nId, vertexCount);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::bindVao(GLuint vao) {
    printf("GfxController::bindVao: VAO ID %d\n", vao);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::setCapability(int capabilityId, bool enabled) {
    printf("GfxController::setCapability: CAPABILITY: %d, enabled: %d",
        capabilityId, enabled);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::initVao(GLuint *vao) {
    printf("GfxController::initVao: vao %p", vao);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::deleteTextures(GLuint *tId) {
    printf("GfxController::initVao: tId %p", tId);
    return GFX_OK(GLuint);
}

GfxResult<GLuint> DummyGfxController::generateFontTextures(GLuint width, GLuint rows, unsigned char *buffer) {
    printf("GfxController::generateFontTextures: width %d, rows %d, buffer %p",
        width, rows, buffer);
    return GFX_OK(GLuint);
}
