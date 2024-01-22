/**
 * @file DummyGfxController.hpp
 * @author Christian Galvez
 * @brief 
 * @version 0.1
 * @date 2024-01-21
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once
#include <GfxController.hpp>

class DummyGfxController : public GfxController {
    GfxResult generateVertexBuffer(Polygon &);
    GfxResult generateNormalBuffer(Polygon &);
    GfxResult generateTextureBuffer(Polygon &, SDL_Surface *);
};