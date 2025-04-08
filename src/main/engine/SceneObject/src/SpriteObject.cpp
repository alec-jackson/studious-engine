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

/// @todo Resolution is hardcoded to 720p right now. Add functionality to change this on the fly. Will need to re-send
/// projection matrix.
void SpriteObject::initializeShaderVars() {
    GameObject2D::initializeShaderVars();
    tintId_ = gfxController_->getShaderVariable(programId_, "tint").get();
}

/// @todo Do something useful here
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

void SpriteObject::splitGrid(int width, int height, int frameCount) {
    // Re-open the image and process it
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
        /* What is the size of each pixel??? */
        data = std::unique_ptr<uint8_t[]>(new uint8_t[width * height * pixelSize]);
        /* This is going to suck, but I can't think of a clever solution.
           Copy each frame line by line... */
        auto imageRow = image->w * height * (i / numHorizontal);
        for (int j = 0; j < height; ++j) {
            /* Select the row of images in the sprite grid */

            auto imageStart = (image->w * j) + imageRow + ((i % numHorizontal) * width);
            memcpy(&data.get()[j * width * pixelSize], &packedData.get()[imageStart * pixelSize], width * pixelSize);
        }

        /* Generate a texture ID with the capture texture data */
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

    // Update the dimensions of the SpriteObject
    GameObject2D::setDimensions(width, height);

    /* GfxController will handle garbage collection of old data */
    GameObject2D::initializeVertexData();

    currentFrame_ = 0;  // Set the current frame to zero as the default
}
