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
    return GfxResult<GLint>(GfxApiResult::OK, 0);
}

GfxResult<GLint> DummyGfxController::generateNormalBuffer(Polygon &polygon) {
    cout << "GfxController::generateNormalBuffer" << endl;
    return GfxResult<GLint>(GfxApiResult::OK, 0);
}

GfxResult<GLint> DummyGfxController::generateTextureBuffer(Polygon &polygon, SDL_Surface *texture) {
    cout << "GfxController::generateTextureBuffer" << endl;
    return GfxResult<GLint>(GfxApiResult::OK, 0);
}

GfxResult<GLint> DummyGfxController::getShaderVariable(GLuint, const char *) {
    cout << "GfxController::getShaderVariable" << endl;
    return GfxResult<GLint>(GfxApiResult::OK, 0);
}

GfxResult<GLint> DummyGfxController::cleanup() {
    cout << "GfxController::cleanup" << endl;
    return GfxResult<GLint>(GfxApiResult::OK, 0);
}

GfxResult<GLuint> DummyGfxController::getProgramId(uint index) {
    cout << "GfxController::getProgramId" << endl;
    return GfxResult<GLuint>(GfxApiResult::OK, 0);
}

GfxResult<GLuint> DummyGfxController::loadShaders(string vertexPath, string fragmentPath) {
    cout << "GfxController::loadShaders" << endl;
    return GfxResult<GLuint>(GfxApiResult::OK, 0);
}

void DummyGfxController::update() {
    cout << "GfxController::update" << endl;
}

GfxResult<GLint> DummyGfxController::init() {
    cout << "GfxController::init" << endl;
    return GfxResult<GLint>(GfxApiResult::OK, 0);
}

GfxResult<GLuint> DummyGfxController::setProgram(GLuint programId) {
    cout << "GfxController::setProgram" << endl;
    return GfxResult<GLuint>(GfxApiResult::OK, 0);
}

GfxResult<GLuint> DummyGfxController::sendFloat(GLuint variableId, GLfloat data) {
    printf("GfxController::sendFloat: variableId=[%u], data=[%f]", variableId, data);
    return GfxResult<GLuint>(GfxApiResult::OK, 0);
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

GfxResult<GLuint> DummyGfxController::render(GLuint vId, GLuint tId, GLuint nId, GLuint vertexCount) {
    printf("GfxController::bindTexture: vId=[%u], tId=[%u], nId=[%u], vertexCount=[%u]",
        vId, tId, nId, vertexCount);
    return GFX_OK(GLuint);
}