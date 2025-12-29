#pragma once
#include <ComplexCameraObject.hpp>

class TPSCameraObject : public ComplexCameraObject {
 public:
    TPSCameraObject(SceneObject *target, vec3 offset, float cameraAngle, float aspectRatio,
        float nearClipping, float farClipping, ObjectType type, string objectName, GfxController *gfxController);
};
