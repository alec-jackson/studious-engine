/**
 * @file FPSCameraObject.hpp
 * @author Christian Galvez
 * @copyright Studious Engine 2025
 * @brief Implementation of the first person camera object.
 * @date 2025-12-28
 */
#include <FPSCameraObject.hpp>
#include <memory>
#include <InputController.hpp>
#include <AnimationController.hpp>
#include <physics.hpp>

extern std::unique_ptr<InputController> inputController;
extern std::unique_ptr<AnimationController> animationController;
extern std::unique_ptr<PhysicsController> physicsController;

/**
 * Note for myself: This is baially going to be the exact same, EXCEPT the lookAt matrix is going to be
 * the player position + OFFSET! This basically swaps the camera and target's position in a way.
 */
FPSCameraObject::FPSCameraObject(SceneObject *target, vec3 offset, vec3 camPos, float cameraAngle, float aspectRatio,
    float nearClipping, float farClipping, ObjectType type, string objectName, GfxController *gfxController) :
    TPSCameraObject(target, offset, cameraAngle, aspectRatio, nearClipping, farClipping, type, objectName,
    gfxController), camPos_ { camPos } {
    // Invert X/Y since camera pos inverted
    invertX = true;
    invertY = true;
    init();
}

void FPSCameraObject::update() {
    if (!headless_) updateInput();
    CameraObject::update();
    assert(target_ != nullptr);
    if (target_ != nullptr) {
        /* If there is a target, aim the camera at it */
        auto eye = target_->getPosition(camPos_);
        // For FPS, the eye is actually going to just be the target's position:

        auto center = target_->getPosition(offset_);
        mat4 projectionMatrix = perspective(radians(cameraAngle_), aspectRatio_,
            nearClipping_, farClipping_);
        // And new lookat target is the offset!
        /* Sprite Objects will use a simple translation matrix instead of a view matrix */
        if (target_->type() == GAME_OBJECT) {
            auto viewMatrix = lookAt(eye, center, vec3(0, 1, 0));
            vpMatrixPerspective_ = projectionMatrix * viewMatrix * rotateMatrix_;
        }
    }
}
