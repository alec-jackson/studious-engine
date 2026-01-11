/**
 * @file ComplexCameraObject.hpp
 * @author Christian Galvez
 * @copyright Studious Engine 2025
 * @brief Intermediary class for FPS/TPS camera shared code.
 * @date 2025-12-28
 */
#pragma once
#include <string>
#include <CameraObject.hpp>
#include <SDL_gamecontroller.h>

#define PI 3.14159265
#define TRACKING_SPEED 20.0f

class ComplexCameraObject : public CameraObject {
 public:
    ComplexCameraObject(SceneObject *target, vec3 offset, float cameraAngle, float aspectRatio,
        float nearClipping, float farClipping, ObjectType type, string objectName, GfxController *gfxController);
    void update() override;
    void sendInput(float xModifier, float yModifier);
    vec3 getDirRay();
 protected:
    // Input related variables will be removed when InputController gains more functionality
    bool hasActiveController = false;
    SDL_GameController *gameController1 = nullptr;
    vec3 cameraOffset;
    vec3 pos;
    bool invertX;
    bool invertY;
    bool ignoreFirstUpdate_ = true;
};
