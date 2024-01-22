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

GfxResult DummyGfxController::generateVertexBuffer(Polygon &polygon) {
    cout << "GfxController::generateVertexBuffer" << endl;
    return GfxResult::OK;
}

GfxResult DummyGfxController::generateNormalBuffer(Polygon &polygon) {
    cout << "GfxController::generateNormalBuffer" << endl;
    return GfxResult::OK;
}

GfxResult DummyGfxController::generateTextureBuffer(Polygon &polygon, SDL_Surface *texture) {
    cout << "GfxController::generateTextureBuffer" << endl;
    return GfxResult::OK;
}
