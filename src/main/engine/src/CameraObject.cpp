#include "CameraObject.hpp"

// TODO: Initializer lists for all SceneObject classes
CameraObject::CameraObject(cameraInfo camInfo) {
    aspectRatio = camInfo.viewAspectRatio;
    nearClipping = camInfo.viewNearClipping;
    farClipping = camInfo.viewFarClipping;
    offset = camInfo.offset;
    target = camInfo.objTarget;
    cameraAngle = camInfo.viewCameraAngle;
}

// TODO: Change NULL checks to nullptr
void CameraObject::updateCamera() {
    if (target == NULL) {
        cerr << "Error: Unable to update camera! Camera target is NULL!\n";
        throw std:runtime_error("Camera target is NULL");
    }
    // Create critical section here to prevent race conditions
    mat4 viewMatrix = lookAt(target->getPos(offset), target->getPos(),
        vec3(0,1,0));
    mat4 projectionMatrix = perspective(radians(cameraAngle), aspectRatio,
        nearClipping, farClipping);
    VPmatrix = projectionMatrix * viewMatrix;
}
