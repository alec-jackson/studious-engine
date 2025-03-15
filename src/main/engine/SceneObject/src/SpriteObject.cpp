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
        string objectName, ObjectType type, ObjectAnchor anchor, GfxController *gfxController): GameObject2D(
            spritePath, position, scale, programId, objectName, type, anchor, gfxController), tint_ { vec4(0) } {
    printf("SpriteObject::SpriteObject: Creating sprite %s\n", objectName.c_str());
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

