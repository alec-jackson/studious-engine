/**
 * @file SpriteObject.cpp
 * @author Christian Galvez
 * @brief Implementation for GameObject2D
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
#include <GameObject2D.hpp>

GameObject2D::GameObject2D(string texturePath, vec3 position, float scale, unsigned int programId,
        string objectName, ObjectType type, ObjectAnchor anchor, GfxController *gfxController): SceneObject(position,
    vec3(0.0f, 0.0f, 0.0f), objectName, scale, programId, type, gfxController), texturePath_ { texturePath },
    anchor_ { anchor } {
    printf("GameObject2D::GameObject2D: Creating 2D object %s\n", objectName.c_str());
}

/// @todo Resolution is hardcoded to 720p right now. Add functionality to change this on the fly. Will need to re-send
/// projection matrix.
void GameObject2D::initializeShaderVars() {
    mat4 projection = ortho(0.0f, static_cast<float>(1280), 0.0f, static_cast<float>(720));
    gfxController_->setProgram(programId_);
    auto projectionId = gfxController_->getShaderVariable(programId_, "projection").get();
    gfxController_->sendFloatMatrix(projectionId, 1, glm::value_ptr(projection));
    modelMatId_ = gfxController_->getShaderVariable(programId_, "model").get();
}

void GameObject2D::initializeTextureData() {
    cout << "GameObject2D::initializeTextureData with path " << texturePath_ << endl;
    SDL_Surface *texture = IMG_Load(texturePath_.c_str());
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

    textureWidth_ = texture->w;
    textureHeight_ = texture->h;

    SDL_FreeSurface(texture);
}

void GameObject2D::initializeVertexData() {
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
            fprintf(stderr, "GameObject2D::initializeVertexData: Unsupported anchor type %d\n", anchor_);
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
    vector<float> vertices = {
        x, y2, 0.0f, 0.0f,
        x, y, 0.0f, 1.0f,
        x2, y2, 1.0f, 0.0f,

        x2, y2, 1.0f, 0.0f,
        x, y, 0.0f, 1.0f,
        x2, y, 1.0f, 1.0f
    };

    // Send VBO data for each character to the currently bound buffer
    gfxController_->sendBufferData(sizeof(float) * vertices.size(), &vertices[0]);
    gfxController_->enableVertexAttArray(0, 4);
    gfxController_->bindBuffer(0);
    gfxController_->bindVao(0);
}

/// @todo Do something useful here
GameObject2D::~GameObject2D() {
}

void GameObject2D::render() {
    printf("GameObject2D::render: Base GameObject2D render called, rendering nothing\n");
}

void GameObject2D::update() {
    render();
}

