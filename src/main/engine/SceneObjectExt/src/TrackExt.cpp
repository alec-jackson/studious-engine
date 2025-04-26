/**
 * @file TrackExt.cpp
 * @brief Function definitions for animation track support.
 * @author Christian Galvez
 * @copyright Copyright (c) 2025
 */

#include <memory>
#include <TrackExt.hpp>
#include <GfxController.hpp>
#include <SDL_image.h>
#include <assert.h>

/**
 * @brief Splits the sprite grid image into multiple equally sized frames. Re-opens the sprite and creates a texture
 * for each frame inside of the sprite grid. Creates frames in a sprite grid in sequential order from top left to
 * bottom right. Will assert if the dimensions of the image will not work.
 * 
 * If any asserts occur when running this function then something about the passed in image is bad. When this function
 * is called, the SpriteObject will no longer render itself as the passed in image. Instead, by default it will render
 * the first frame in the sprite grid and re-size the object itself to the dimensions of the first frame.
 * 
 * The image's width must be perfectly divisible by the width of each frame. The same is true for the height. The passed
 * in frameCount must also be less than or equal to the number of possible frames in the image given the width and height
 * of each frame.
 * 
 * @param width Of each frame in the sprite grid.
 * @param height Of each frame in the sprite grid.
 * @param frameCount The number of frames to pull from the sprite grid.
 */
void TrackExt::splitGrid(int width, int height, int frameCount) {
    /* Re-open the image and process it */
    auto image = IMG_Load(imagePath_.c_str());
    if (image == nullptr) {
        fprintf(stderr, "SpriteObject::splitGrid: Error - unable to open image %s\n",
            imagePath_.c_str());
        assert(0);
        return;
    }

    /* Validate the width, height and frame count */
    assert(image->w % width == 0);
    assert(image->h % height == 0);

    /* No use in having a zero frame count, right? */
    assert(frameCount > 0);

    /* Detect the max frame count from the image dimensions */
    auto numHorizontal = image->w / width;
    auto numVertical = image->h / height;
    auto maxFrames = numHorizontal * numVertical;

    /* Determine the size of each pixel */
    auto pixelSize = image->format->BytesPerPixel;
    auto imageFormat = image->format->Amask ? TexFormat::RGBA : TexFormat::RGB;
    assert(frameCount <= maxFrames);
    std::unique_ptr<uint8_t[]> data;
    imageBank_.width = width;
    imageBank_.height = height;
    auto packedData = packSurface(image);

    /* Grab frames LEFT TO RIGHT from image data */
    for (int i = 0; i < frameCount; ++i) {
        data = std::unique_ptr<uint8_t[]>(new uint8_t[width * height * pixelSize]);
        /* This is going to suck, but I can't think of a clever solution.
           Copy each frame line by line... */
        auto imageRow = image->w * height * (i / numHorizontal);
        for (int j = 0; j < height; ++j) {
            /* Select the row of images in the sprite grid */

            auto imageStart = (image->w * j) + imageRow + ((i % numHorizontal) * width);
            memcpy(&data.get()[j * width * pixelSize], &packedData.get()[imageStart * pixelSize], width * pixelSize);
        }

        /* Create a texture for the current frame */
        unsigned int textureId;

        extGfx_->generateTexture(&textureId);
        extGfx_->bindTexture(textureId);
        extGfx_->sendTextureData(width, height, imageFormat, data.get());
        extGfx_->setTexParam(TexParam::WRAP_MODE_S, TexVal(TexValType::CLAMP_TO_EDGE));
        extGfx_->setTexParam(TexParam::WRAP_MODE_T, TexVal(TexValType::CLAMP_TO_EDGE));
        extGfx_->setTexParam(TexParam::MAGNIFICATION_FILTER, TexVal(TexValType::NEAREST_NEIGHBOR));
        extGfx_->setTexParam(TexParam::MINIFICATION_FILTER, TexVal(TexValType::NEAREST_MIPMAP));
        extGfx_->setTexParam(TexParam::MIPMAP_LEVEL, TexVal(10));
        extGfx_->generateMipMap();

        /* Add the current image to the image bank */
        imageBank_.textureIds.push_back(textureId);
    }

    SDL_FreeSurface(image);

    currentFrame_ = 0;  // Set the current frame to zero as the default
}

/**
 * @brief Tightly packs texture data stored in an SDL_Surface to remove 4-byte alignment.
 * 
 * @param texture Valid SDL_Surface containing image data.
 * @return std::shared_ptr<uint8_t[]> Buffer containing tightly packed pixel data.
 */
std::shared_ptr<uint8_t[]> TrackExt::packSurface(SDL_Surface *texture) {
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