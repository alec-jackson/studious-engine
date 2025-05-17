/**
 * @file Image.hpp
 * @author Christian Galvez
 * @brief Image class definition for storing image data
 * @version 0.1
 * @date 2024-10-20
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include <SDL2/SDL_image.h>
#include <vector>
#include <memory>

struct Image {
    /* Constraints across all image resolutions */
    int width;
    int height;

    /* textureIds for each frame */
    std::vector<unsigned int> textureIds;
};

/**
 * @brief Tightly packs texture data stored in an SDL_Surface to remove 4-byte alignment.
 *
 * @param texture Valid SDL_Surface containing image data.
 * @return std::shared_ptr<uint8_t[]> Buffer containing tightly packed pixel data.
 */
std::shared_ptr<uint8_t[]> packSurface(SDL_Surface *texture);

SDL_Surface *convertSurfaceToRgba(SDL_Surface *surface);
