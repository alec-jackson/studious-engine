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

CameraObject::CameraObject(cameraInfo camInfo): SceneObject(), aspectRatio(camInfo.viewAspectRatio),
    nearClipping(camInfo.viewNearClipping), farClipping(camInfo.viewFarClipping), offset(camInfo.offset),
    target(camInfo.objTarget), cameraAngle(camInfo.viewCameraAngle) {}

/// @todo: Figure out what the destructor should do
CameraObject::~CameraObject() {
}

/// @todo: Change NULL checks to nullptr
void CameraObject::render() {
    if (target == NULL) {
        cerr << "Error: Unable to update camera! Camera target is NULL!\n";
        throw std::runtime_error("Camera target is NULL");
    }
    // Create critical section here to prevent race conditions
    mat4 viewMatrix = lookAt(target->getPosition(offset), target->getPosition(),
        vec3(0, 1, 0));
    mat4 projectionMatrix = perspective(radians(cameraAngle), aspectRatio,
        nearClipping, farClipping);
    vpMatrix = projectionMatrix * viewMatrix;
}
