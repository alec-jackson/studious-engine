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
#include <GameObject2D.hpp>
#include <string>
#include <cstdio>
#include <iostream>
#include <vector>
#include <memory>

GameObject2D::GameObject2D(string texturePath, vec3 position, float scale, unsigned int programId,
    string objectName, ObjectType type, ObjectAnchor anchor, GfxController *gfxController): SceneObject(position,
    vec3(0.0f, 0.0f, 0.0f), scale, programId, type, objectName, gfxController),
    TrackExt(texturePath, this, gfxController), texturePath_ { texturePath },
    anchor_ { anchor } {
    printf("GameObject2D::GameObject2D: Creating 2D object %s\n", objectName.c_str());
}

void GameObject2D::initializeShaderVars() {
    gfxController_->setProgram(programId_);
    projectionId_ = gfxController_->getShaderVariable(programId_, "projection").get();
    modelMatId_ = gfxController_->getShaderVariable(programId_, "model").get();
}

void GameObject2D::initializeTextureData() {
    cout << "GameObject2D::initializeTextureData with path " << texturePath_ << endl;
    SDL_Surface *texture = IMG_Load(texturePath_.c_str());
    if (texture == nullptr) {
        fprintf(stderr, "GameObject2D::initializeTextureData: Failed to load texture %s\n", texturePath_.c_str());
        return;
    }
    auto textureFormat = texture->format->Amask ? TexFormat::RGBA : TexFormat::RGB;
    auto packedData = packSurface(texture);
    // Send texture image to OpenGL
    gfxController_->generateTexture(&textureId_);
    gfxController_->bindTexture(textureId_, GfxTextureType::NORMAL);
    gfxController_->sendTextureData(texture->w, texture->h, textureFormat, packedData.get());
    gfxController_->setTexParam(TexParam::WRAP_MODE_S, TexVal(TexValType::CLAMP_TO_EDGE), GfxTextureType::NORMAL);
    gfxController_->setTexParam(TexParam::WRAP_MODE_T, TexVal(TexValType::CLAMP_TO_EDGE), GfxTextureType::NORMAL);
    gfxController_->setTexParam(TexParam::MAGNIFICATION_FILTER, TexVal(TexValType::NEAREST_NEIGHBOR),
        GfxTextureType::NORMAL);
    gfxController_->setTexParam(TexParam::MINIFICATION_FILTER, TexVal(TexValType::NEAREST_MIPMAP),
        GfxTextureType::NORMAL);
    gfxController_->setTexParam(TexParam::MIPMAP_LEVEL, TexVal(10), GfxTextureType::NORMAL);
    gfxController_->generateMipMap();

    textureWidth_ = texture->w;
    textureHeight_ = texture->h;

    SDL_FreeSurface(texture);
}

/**
 * @brief Updates the dimensions of the sprite. When using the splitGrid function, the image
 * itself will decrease in size, so we want to account for that. Run this to ensure the pixel
 * size remains the same after spliting.
 *
 * @param width Width of the new sprite grid frame.
 * @param height Height of the new sprite grid frame.
 */
void GameObject2D::setDimensions(int width, int height) {
    textureWidth_ = width;
    textureHeight_ = height;
}

/// @todo Do something useful here
GameObject2D::~GameObject2D() {
}

void GameObject2D::render() {
    VISIBILITY_CHECK;
    printf("GameObject2D::render: Base GameObject2D render called, rendering nothing\n");
}

void GameObject2D::update() {
    render();
}

/**
 * @brief Creates a collider for this game object
 *
 * @param programId Program used to render the collider (collider shaders)
 */
void GameObject2D::createCollider() {
    printf("GameObject2D::createCollider: Creating collider for object %s\n", objectName.c_str());
    auto colliderName = objectName + "-Collider";
    auto colliderProg = gfxController_->getProgramId(COLLIDEROBJECT_PROG_NAME);
    if (!colliderProg.isOk()) {
        fprintf(stderr,
            "GameObject2D::createCollider: Failed to create collider! '%s' program does not exist!\n",
            COLLIDEROBJECT_PROG_NAME);
        return;
    }
    collider_ = std::make_shared<ColliderObject>(vertTexData_, colliderProg.get(), &translateMatrix_, &scaleMatrix_,
        &vpMatrix_, ObjectType::GAME_OBJECT, colliderName, gfxController_);
}

/**
 * @brief Get the collider for the 2D Game Object
 */
ColliderObject *GameObject2D::getCollider() {
    collider_.get()->updateCollider();
    return collider_.get();
}
