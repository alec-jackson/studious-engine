#pragma once
#include <TPSCameraObject.hpp>
#include <SDL_gamecontroller.h>

class FPSCameraObject : public TPSCameraObject {
 public:
    FPSCameraObject(SceneObject *target, vec3 offset, vec3 camPos, float cameraAngle, float aspectRatio,
        float nearClipping, float farClipping, ObjectType type, string objectName, GfxController *gfxController);
    void update() override;
 protected:
    vec3 camPos_;
};
