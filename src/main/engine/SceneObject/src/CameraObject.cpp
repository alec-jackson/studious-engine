/**
 * @file CameraObject.cpp
 * @author Christian Galvez
 * @brief Implementation for CameraObject
 * @version 0.1
 * @date 2023-07-28
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <CameraObject.hpp>
#include <string>
#include <cstdio>
#include <algorithm>
#include <vector>

CameraObject::CameraObject(SceneObject *target, vec3 offset, float cameraAngle, float aspectRatio,
    float nearClipping, float farClipping, ObjectType type, string objectName, GfxController *gfxController) :
    SceneObject(type, objectName, gfxController), target_ { target }, offset_ { offset }, cameraAngle_ { cameraAngle },
    aspectRatio_ { aspectRatio }, nearClipping_ { nearClipping }, farClipping_ { farClipping } {
        initialTargetPos_ = target != nullptr ? target->getPosition() : vec3(0);
    }

/// @todo: Figure out what the destructor should do
CameraObject::~CameraObject() {
}

void CameraObject::render() {
    vec3 eye = vec3(0);
    vec3 center = vec3(0.0f, 0.01f, 0.0f);
    orthographicMatrix_ = ortho(0.0f, 1280.0f, 0.0f, 720.0f, -1.0f, 1.0f);
    mat4 projectionMatrix = perspective(radians(cameraAngle_), aspectRatio_,
        nearClipping_, farClipping_);
    mat4 viewMatrix(1.0f);
    rotateMatrix_ = glm::rotate(mat4(1.0f), glm::radians(rotation[0]),
            vec3(1, 0, 0))  *glm::rotate(mat4(1.0f), glm::radians(rotation[1]),
            vec3(0, 1, 0))  *glm::rotate(mat4(1.0f), glm::radians(rotation[2]),
            vec3(0, 0, 1));
    vpMatrixOrthographic_ = orthographicMatrix_;
    if (target_ != nullptr) {
        /* If there is a target, aim the camera at it */
        eye = target_->getPosition(offset_);
        center = target_->getPosition(center);
        /* Sprite Objects will use a simple translation matrix instead of a view matrix */
        if (target_->type() == SPRITE_OBJECT) {
            viewMatrix = glm::translate(glm::mat4(1.0f), vec3(-1) * (target_->getPosition() - initialTargetPos_));
            vpMatrixOrthographic_ = orthographicMatrix_ * viewMatrix;
        } else {
            viewMatrix = lookAt(eye, center, vec3(0, 1, 0));
        }
    }
    vpMatrixPerspective_ = projectionMatrix * viewMatrix * rotateMatrix_;
}

void CameraObject::update() {
    // Update aspect ratio
    setAspectRatio(resolution_.x / resolution_.y);
    render();
}
