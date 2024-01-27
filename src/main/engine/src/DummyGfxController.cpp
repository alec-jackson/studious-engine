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

GfxResult<GLint> DummyGfxController::getShaderVariable(GLint, const char *) const {
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