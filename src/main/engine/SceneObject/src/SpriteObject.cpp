/**
 * @file SpriteObject.cpp
 * @author Christian Galvez
 * @brief Implementation for SpriteObject
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <string>
#include <cstdio>
#include <iostream>
#include <vector>
#include <memory>
#include <SpriteObject.hpp>

SpriteObject::SpriteObject(string spritePath, vec3 position, float scale, unsigned int programId,
        string objectName, ObjectType type, ObjectAnchor anchor, GfxController *gfxController): GameObject2D(
            spritePath, position, scale, programId, objectName, type, anchor, gfxController), tint_ { vec4(0) } {
    printf("SpriteObject::SpriteObject: Creating sprite %s\n", objectName.c_str());
    GameObject2D::initializeTextureData();
    GameObject2D::initializeVertexData();
    initializeShaderVars();
}

void SpriteObject::initializeShaderVars() {
    GameObject2D::initializeShaderVars();
    tintId_ = gfxController_->getShaderVariable(programId_, "tint").get();
}

SpriteObject::~SpriteObject() {
}

void SpriteObject::render() {
    translateMatrix_ = glm::translate(mat4(1.0f), position);
    rotateMatrix_ = glm::rotate(mat4(1.0f), glm::radians(rotation[0]),
            vec3(1, 0, 0))  *glm::rotate(mat4(1.0f), glm::radians(rotation[1]),
            vec3(0, 1, 0))  *glm::rotate(mat4(1.0f), glm::radians(rotation[2]),
            vec3(0, 0, 1));
    scaleMatrix_ = glm::scale(vec3(scale, scale, scale));
    modelMat_ = translateMatrix_ * rotateMatrix_ * scaleMatrix_;
    gfxController_->clear(GfxClearMode::DEPTH);
    gfxController_->setProgram(programId_);
    gfxController_->polygonRenderMode(RenderMode::FILL);
    // Send shader variables
    gfxController_->sendFloatMatrix(modelMatId_, 1, glm::value_ptr(modelMat_));
    gfxController_->sendFloatVector(tintId_, 1, glm::value_ptr(tint_));
    gfxController_->sendFloatMatrix(projectionId_, 1, glm::value_ptr(vpMatrix_));
    // Find a more clever solution
    gfxController_->bindVao(vao_);
    /* Bind the texture based on the sprite grid split */
    if (imageBank_.textureIds.empty()) {
        /* Send the base image if no images are present in the image bank */
        gfxController_->bindTexture(textureId_);
    } else {
        assert(currentFrame_ < imageBank_.textureIds.size());
        gfxController_->bindTexture(imageBank_.textureIds.at(currentFrame_));
    }
    gfxController_->drawTriangles(6);
    gfxController_->bindVao(0);
    gfxController_->bindTexture(0);
    if (collider_.use_count() > 0) collider_.get()->update();
}

void SpriteObject::update() {
    render();
}

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
void SpriteObject::splitGrid(int width, int height, int frameCount) {
    /* Re-open the image and process it */
    auto image = IMG_Load(texturePath_.c_str());
    if (image == nullptr) {
        fprintf(stderr, "SpriteObject::splitGrid: Error - unable to open image %s\n",
            texturePath_.c_str());
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
    auto packedData = GameObject2D::packSurface(image);

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

        gfxController_->generateTexture(&textureId);
        gfxController_->bindTexture(textureId);
        gfxController_->sendTextureData(width, height, imageFormat, data.get());
        gfxController_->setTexParam(TexParam::WRAP_MODE_S, TexVal(TexValType::CLAMP_TO_EDGE));
        gfxController_->setTexParam(TexParam::WRAP_MODE_T, TexVal(TexValType::CLAMP_TO_EDGE));
        gfxController_->setTexParam(TexParam::MAGNIFICATION_FILTER, TexVal(TexValType::NEAREST_NEIGHBOR));
        gfxController_->setTexParam(TexParam::MINIFICATION_FILTER, TexVal(TexValType::NEAREST_MIPMAP));
        gfxController_->setTexParam(TexParam::MIPMAP_LEVEL, TexVal(10));
        gfxController_->generateMipMap();

        /* Add the current image to the image bank */
        imageBank_.textureIds.push_back(textureId);
    }

    SDL_FreeSurface(image);

    /* Update the dimensions of the SpriteObject to match the frame size */
    GameObject2D::setDimensions(width, height);

    /* GfxController will handle garbage collection of old data */
    GameObject2D::initializeVertexData();

    currentFrame_ = 0;  // Set the current frame to zero as the default
}
