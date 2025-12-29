#pragma once
#include <CameraObject.hpp>
#include <SDL_gamecontroller.h>

const int JOYSTICK_DEAD_ZONE = 4000;
#define PI 3.14159265
#define TRACKING_SPEED 0.1f

class TPSCameraObject : public CameraObject {
 public:
    TPSCameraObject(SceneObject *target, vec3 offset, float cameraAngle, float aspectRatio,
        float nearClipping, float farClipping, ObjectType type, string objectName, GfxController *gfxController);
    void update() override;
    void updateInput();
    void init();
    inline void setHeadless(bool headless) { headless_ = headless; }
    inline bool headless() { return headless_; }
    vec3 getDirRay();
 protected:
    // Input related variables will be removed when InputController gains more functionality
    bool hasActiveController = false;
    bool headless_ = false;
    SDL_GameController *gameController1 = nullptr;
    vec3 cameraOffset;
    vec3 angles;
    vec3 pos;
    bool invertX = false;
    bool invertY = false;
};
