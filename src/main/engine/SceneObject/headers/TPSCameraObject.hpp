/**
 * @file TPSCameraObject.hpp
 * @author Christian Galvez
 * @copyright Studious Engine 2025
 * @brief Declaration of the third person camera object for studious.
 * @date 2025-12-28
 */
#pragma once
#include <string>
#include <ComplexCameraObject.hpp>

class TPSCameraObject : public ComplexCameraObject {
 public:
    TPSCameraObject(SceneObject *target, vec3 offset, float cameraAngle, float aspectRatio,
        float nearClipping, float farClipping, ObjectType type, string objectName, GfxController *gfxController);
};
