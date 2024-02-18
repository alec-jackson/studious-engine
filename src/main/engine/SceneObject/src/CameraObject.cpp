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

CameraObject::CameraObject(GameObject *target, vec3 offset, GLfloat cameraAngle, GLfloat aspectRatio,
    GLfloat nearClipping, GLfloat farClipping, GfxController *gfxController) : SceneObject(gfxController),
    target_ { target }, offset_ { offset }, cameraAngle_ { cameraAngle }, aspectRatio_ { aspectRatio },
    nearClipping_ { nearClipping }, farClipping_ { farClipping } {}

/// @todo: Figure out what the destructor should do
CameraObject::~CameraObject() {
}

/// @todo: Change NULL checks to nullptr
void CameraObject::render() {
    /// @todo Add field to modify target offset
    mat4 viewMatrix = lookAt(target_->getPosition(offset_), target_->getPosition(vec3(0.0f, 0.01f, 0.0f)),
        vec3(0, 1, 0));
    mat4 projectionMatrix = perspective(radians(cameraAngle_), aspectRatio_,
        nearClipping_, farClipping_);
    vpMatrix_ = projectionMatrix * viewMatrix;
}
