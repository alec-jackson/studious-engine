/**
 * @file TrackExt.hpp
 * @author Christian Galvez
 * @brief Extension that allows for track animations
 * @copyright Copyright (c) 2025
 */

#pragma once
#include <string>
#include <SDL_image.h>
#include <Image.hpp>
#include <GfxController.hpp>
class TrackExt {
 public:
    inline explicit TrackExt(std::string imagePath, GfxController *gfxController) : imagePath_ { imagePath },
        extGfx_ { gfxController } {}

    // Getters
    inline uint getBankSize() { return imageBank_.textureIds.size(); }
    inline uint getCurrentFrame() { return currentFrame_; }

    // Setters
    inline void setCurrentFrame(int frame) { currentFrame_ = frame; }
 protected:
    void splitGrid(int width, int height, int frameCount);
    std::shared_ptr<uint8_t[]> packSurface(SDL_Surface *texture);
    Image imageBank_;
    uint currentFrame_;
    std::string imagePath_;
 private:
    GfxController *extGfx_;
};