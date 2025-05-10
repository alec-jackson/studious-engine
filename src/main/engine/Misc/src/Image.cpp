/**
 * @file Image.cpp
 * @brief Helper functions for managing texture data.
 * @author Christian Galvez
 * @date May 9, 2025
 * @copyright Studious-Engine 2025
 */
#include <Image.hpp>
#include <memory>

std::shared_ptr<uint8_t[]> packSurface(SDL_Surface *texture) {
    /* Tightly pack to remove 4 byte alignment on texture */
    auto pixelSize = texture->format->BytesPerPixel;
    std::shared_ptr<uint8_t[]> packedData(new uint8_t[texture->w * texture->h * pixelSize],
        std::default_delete<uint8_t[]>());
    for (int i = 0; i < texture->h; ++i) {
        memcpy(&packedData.get()[i * pixelSize * texture->w],
            &(reinterpret_cast<uint8_t *>(texture->pixels))[i * (texture->pitch)],
            pixelSize * texture->w);
    }
    return packedData;
}
