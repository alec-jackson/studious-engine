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
#include <GfxController.hpp>
#include <Polygon.hpp>
#include <common.hpp>

class OpenGlGfxController : public GfxController {
 public:
    GfxResult generateVertexBuffer(Polygon &);
    GfxResult generateNormalBuffer(Polygon &);
    GfxResult generateTextureBuffer(Polygon &, SDL_Surface *);
};
