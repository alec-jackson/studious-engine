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
#include <SpriteObject.hpp>

SpriteObject::SpriteObject(string spritePath, vec3 position, float scale, unsigned int programId,
        string objectName, ObjectType type, SpriteAnchor anchor, GfxController *gfxController): SceneObject(position,
    vec3(0.0f, 0.0f, 0.0f), objectName, scale, programId, type, gfxController), spritePath_ { spritePath },
    tint_ { vec4(0) }, anchor_ { anchor } {
    printf("SpriteObject::SpriteObject: Creating sprite %s\n", objectName.c_str());
    initializeShaderVars();
    initializeSprite();
}

/// @todo Resolution is hardcoded to 720p right now. Add functionality to change this on the fly. Will need to re-send
/// projection matrix.
void SpriteObject::initializeShaderVars() {
    mat4 projection = ortho(0.0f, static_cast<float>(1280), 0.0f, static_cast<float>(720));
    gfxController_->setProgram(programId_);
    auto projectionId = gfxController_->getShaderVariable(programId_, "projection").get();
    gfxController_->sendFloatMatrix(projectionId, 1, glm::value_ptr(projection));
    modelMatId_ = gfxController_->getShaderVariable(programId_, "model").get();
    tintId_ = gfxController_->getShaderVariable(programId_, "tint").get();
}

void SpriteObject::initializeSprite() {
    cout << "SpriteObject::initializeSprite with path " << spritePath_ << endl;
    SDL_Surface *texture = IMG_Load(spritePath_.c_str());
    auto textureFormat = texture->format->Amask ? TexFormat::RGBA : TexFormat::RGB;
    // Send texture image to OpenGL
    gfxController_->generateTexture(&textureId_);
    gfxController_->bindTexture(textureId_);
    gfxController_->sendTextureData(texture->w, texture->h, textureFormat, texture->pixels);
    gfxController_->setTexParam(TexParam::WRAP_MODE_S, TexVal(TexValType::CLAMP_TO_EDGE));
    gfxController_->setTexParam(TexParam::WRAP_MODE_T, TexVal(TexValType::CLAMP_TO_EDGE));
    gfxController_->setTexParam(TexParam::MAGNIFICATION_FILTER, TexVal(TexValType::NEAREST_NEIGHBOR));
    gfxController_->setTexParam(TexParam::MINIFICATION_FILTER, TexVal(TexValType::NEAREST_MIPMAP));
    gfxController_->setTexParam(TexParam::MIPMAP_LEVEL, TexVal(10));
    gfxController_->generateMipMap();

    // Perform anchor points here
    auto x = 0.0f, y = 0.0f;
    switch (anchor_) {
        case BOTTOM_LEFT:
            x = 0.0f;
            y = 0.0f;
            break;
        case CENTER:
            x = -1 * ((texture->w) / 2.0f);
            y = (texture->h) / 2.0f;
            break;
        default:
            fprintf(stderr, "SpriteObject::initializeSprite: Unsupported anchor type %d\n", anchor_);
            assert(false);
            break;
    }
    auto x2 = x + (texture->w), y2 = y - (texture->h);
    // Use textures to create each character as an independent object
    gfxController_->initVao(&vao_);
    gfxController_->bindVao(vao_);
    gfxController_->generateBuffer(&vbo_);
    gfxController_->bindBuffer(vbo_);
    // update VBO for each character
    vector<float> vertices = {
        x, y, 0.0f, 0.0f,
        x, y2, 0.0f, 1.0f,
        x2, y2, 1.0f, 1.0f,

        x, y, 0.0f, 0.0f,
        x2, y2, 1.0f, 1.0f,
        x2, y, 1.0f, 0.0f
    };

    // Send VBO data for each character to the currently bound buffer
    gfxController_->sendBufferData(sizeof(float) * vertices.size(), &vertices[0]);
    gfxController_->enableVertexAttArray(0, 4);
    gfxController_->bindBuffer(0);
    gfxController_->bindVao(0);
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
    // Find a more clever solution
    gfxController_->bindVao(vao_);
    gfxController_->bindTexture(textureId_);
    gfxController_->drawTriangles(6);
    gfxController_->bindVao(0);
    gfxController_->bindTexture(0);
}

void SpriteObject::update() {
    render();
}

