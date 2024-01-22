/**
 * @file GfxController.hpp
 * @author Christian Galvez
 * @brief 
 * @version 0.1
 * @date 2024-01-21
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once

#include <Polygon.hpp>

enum GfxResult {
    OK,
    FAILURE
};

class GfxController {
 public:
    virtual GfxResult generateVertexBuffer(Polygon &) = 0;
    virtual GfxResult generateNormalBuffer(Polygon &) = 0;
    virtual GfxResult generateTextureBuffer(Polygon &, SDL_Surface *) = 0;
};
