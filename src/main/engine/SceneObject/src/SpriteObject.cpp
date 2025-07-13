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
#include <cstdio>
#include <vector>
#include <memory>
#include <string>
#include <SpriteObject.hpp>

SpriteObject::SpriteObject(string spritePath, vec3 position, float scale, unsigned int programId,
        string objectName, ObjectType type, ObjectAnchor anchor, GfxController *gfxController): GameObject2D(
            spritePath, position, scale, programId, objectName, type, anchor, gfxController), tint_ { vec3(0) } {
    printf("SpriteObject::SpriteObject: Creating sprite %s\n", objectName.c_str());
    GameObject2D::initializeTextureData();
    initializeVertexData();
    initializeShaderVars();
}

void SpriteObject::initializeShaderVars() {
    GameObject2D::initializeShaderVars();
    tintId_ = gfxController_->getShaderVariable(programId_, "tint").get();
}

void SpriteObject::initializeVertexData() {
    // Perform anchor points here
    auto x = 0.0f, y = 0.0f;
    switch (anchor_) {
        case BOTTOM_LEFT:
            x = 0.0f;
            y = 0.0f;
            break;
        case CENTER:
            x = -1 * ((textureWidth_) / 2.0f);
            y = -1 * ((textureHeight_) / 2.0f);
            break;
        case TOP_LEFT:
            y = -1.0f * textureHeight_;
            x = 0.0f;
            break;
        default:
            fprintf(stderr, "SpriteObject::initializeVertexData: Unsupported anchor type %d\n", anchor_);
            assert(false);
            break;
    }
    auto x2 = x + (textureWidth_), y2 = y + (textureHeight_);
    // Use textures to create each character as an independent object
    gfxController_->initVao(&vao_);
    gfxController_->bindVao(vao_);
    gfxController_->generateBuffer(&vbo_);
    gfxController_->bindBuffer(vbo_);
    // update VBO for each character
    // UV coordinate origin STARTS in the TOP left, NOT BOTTOM LEFT!!!
    vertTexData_ = {
        x, y2, 0.0f, 0.0f,
        x, y, 0.0f, 1.0f,
        x2, y2, 1.0f, 0.0f,

        x2, y2, 1.0f, 0.0f,
        x, y, 0.0f, 1.0f,
        x2, y, 1.0f, 1.0f
    };

    // Send VBO data for each character to the currently bound buffer
    gfxController_->sendBufferData(sizeof(float) * vertTexData_.size(), &vertTexData_[0]);
    gfxController_->enableVertexAttArray(0, 4, sizeof(float), 0);
    gfxController_->bindBuffer(0);
    gfxController_->bindVao(0);
}

SpriteObject::~SpriteObject() {
}

void SpriteObject::render() {
    updateModelMatrices();
    mat4 model = translateMatrix_ * rotateMatrix_ * scaleMatrix_;
    gfxController_->clear(GfxClearMode::DEPTH);
    gfxController_->setProgram(programId_);
    gfxController_->polygonRenderMode(RenderMode::FILL);
    // Send shader variables
    gfxController_->sendFloatMatrix(modelMatId_, 1, glm::value_ptr(model));
    gfxController_->sendFloatVector(tintId_, 1, VectorType::GFX_3D, glm::value_ptr(tint_));
    gfxController_->sendFloatMatrix(projectionId_, 1, glm::value_ptr(vpMatrix_));
    // Find a more clever solution
    gfxController_->bindVao(vao_);
    /* Bind the texture based on the sprite grid split */
    if (imageBank_.textureIds.empty()) {
        /* Send the base image if no images are present in the image bank */
        gfxController_->bindTexture(textureId_, GfxTextureType::NORMAL);
    } else {
        assert(currentFrame_ < imageBank_.textureIds.size());
        gfxController_->bindTexture(imageBank_.textureIds.at(currentFrame_), GfxTextureType::NORMAL);
    }
    gfxController_->drawTriangles(6);
    gfxController_->bindVao(0);
    gfxController_->bindTexture(0, GfxTextureType::NORMAL);
    if (collider_.use_count() > 0) collider_.get()->update();
}

void SpriteObject::update() {
    render();
}

void SpriteObject::createAnimation(int width, int height, int frameCount) {
    splitGrid(width, height, frameCount);

    // Update GameObject2D dimensions

    /* Update the dimensions of the SpriteObject to match the frame size */
    setDimensions(width, height);

    /* GfxController will handle garbage collection of old data */
    initializeVertexData();
}
