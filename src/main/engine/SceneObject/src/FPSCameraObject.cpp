#include "TPSCameraObject.hpp"
#include <FPSCameraObject.hpp>

#include <InputController.hpp>
#include <AnimationController.hpp>
#include <physics.hpp>
#include <memory>
#include <thread>

extern std::unique_ptr<InputController> inputController;
extern std::unique_ptr<AnimationController> animationController;
extern std::unique_ptr<PhysicsController> physicsController;

/**
 * Note for myself: This is baially going to be the exact same, EXCEPT the lookAt matrix is going to be
 * the player position + OFFSET! This basically swaps the camera and target's position in a way.
 */
FPSCameraObject::FPSCameraObject(SceneObject *target, vec3 offset, vec3 camPos, float cameraAngle, float aspectRatio,
    float nearClipping, float farClipping, ObjectType type, string objectName, GfxController *gfxController) :
    TPSCameraObject(target, offset, cameraAngle, aspectRatio, nearClipping, farClipping, type, objectName, gfxController), camPos_ { camPos } {
        // Do some extra stuff in addition to normal camera work
        // Spin up a thread for input monitoring?
    init();
}

void FPSCameraObject::update() {
    if (!headless_) updateInput();
    cout << "This is a test!" << endl;
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
    assert(target_ != nullptr);
    if (target_ != nullptr) {
        /* If there is a target, aim the camera at it */
        eye = target_->getPosition(camPos_);
        //eye = vec3(0);
        // For FPS, the eye is actually going to just be the target's position:

        center = target_->getPosition(offset_);
        //center = vec3(-0.1, 0, 0);
        // And new lookat target is the offset!
        /* Sprite Objects will use a simple translation matrix instead of a view matrix */
        if (target_->type() == SPRITE_OBJECT) {
            viewMatrix = glm::translate(glm::mat4(1.0f), vec3(-1) * (target_->getPosition() - initialTargetPos_));
            vpMatrixOrthographic_ = orthographicMatrix_ * viewMatrix;
        } else {
            viewMatrix = lookAt(eye, center, vec3(0, 1, 0));
        }
    }
    vpMatrixPerspective_ = projectionMatrix * viewMatrix * rotateMatrix_;
    //CameraObject::update();
    // TODO: Don't copy/paste the code and do it properly (once this works!)
}
