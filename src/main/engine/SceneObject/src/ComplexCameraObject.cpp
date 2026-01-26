/**
 * @file ComplexCameraObject.cpp
 * @author Christian Galvez
 * @copyright Studious Engine 2025
 * @date 2025-12-28
 * @brief Implementation of the FPS/TPS intermediary class code. This class handles input from the InputController
 * and updates the camera's EYE and CENTER values on each update.
 */
#include <ComplexCameraObject.hpp>
#include <memory>
#include <glm/geometric.hpp>
#include <TPSCameraObject.hpp>
#include <InputController.hpp>
#include <AnimationController.hpp>
#include <physics.hpp>

extern std::unique_ptr<InputController> inputController;
extern std::unique_ptr<AnimationController> animationController;
extern std::unique_ptr<PhysicsController> physicsController;
extern double deltaTime;

#define INVERT_MODIFIER(flag) if (flag) modifier *= -1.0f
#define TRACK_TRANSFORM TRACKING_SPEED * modifier

ComplexCameraObject::ComplexCameraObject(SceneObject *target, vec3 offset, float cameraAngle, float aspectRatio,
    float nearClipping, float farClipping, ObjectType type, string objectName, GfxController *gfxController) :
    CameraObject(target, offset, cameraAngle, aspectRatio, nearClipping, farClipping, type, objectName, gfxController) {
}

void ComplexCameraObject::update() {
    CameraObject::update();
}

void ComplexCameraObject::sendInput(float xModifier, float yModifier) {
    /*
    (vector<float>) cameraDistance takes a 3D vector containing the offset of the
    camera from the object and calculates the distance between the two with
    respect to the y-z plane, and x-y plane. Returns a 2D vector where the first
    element is the y-z distance, and the second element is the x-y distance.
    */
    auto cameraDistance = [](vec3 offset) {
        vector<float> distance(2);
        distance[0] = offset[1] * offset[1] + offset[2] * offset[2];
        distance[1] = offset[1] * offset[1] + offset[0] * offset[0];
        return distance;
    };

    assert(nullptr != getTarget());  // Must have a target
    cameraOffset = offset_;
    // Float equal might be hairy, let's see how it works out
    if (yModifier != 0.0f) {
        float modifier = yModifier;
        INVERT_MODIFIER(invertY);
        vector<float> distHold = cameraDistance(cameraOffset);
        cameraOffset[1] += TRACK_TRANSFORM;
        vector<float> distFinish = cameraDistance(cameraOffset);
        distHold[0] = sqrt(distHold[0]);
        distHold[1] = sqrt(distHold[1]);
        distFinish[0] = sqrt(distFinish[0]);
        distFinish[1] = sqrt(distFinish[1]);
        distFinish[0] /= distHold[0];
        distFinish[1] /= distHold[1];
        cameraOffset[1] /= ((distFinish[0] + distFinish[1]) / 2.0f);
        cameraOffset[2] /= distFinish[0];
        cameraOffset[0] /= distFinish[1];
    }
    // Maybe we can remove the conditional? Might hurt performance though...
    if (xModifier != 0.0f) {
        float distHold = cameraOffset[0] * cameraOffset[0] + cameraOffset[2] * cameraOffset[2];
        float modifier = xModifier;
        INVERT_MODIFIER(invertX);
        if (cameraOffset[0] <= pos[0] && cameraOffset[2] <= pos[2]) {
            cameraOffset[0] -= TRACK_TRANSFORM;
            cameraOffset[2] += TRACK_TRANSFORM;
        } else if (cameraOffset[0] <= pos[0] && cameraOffset[2] >= pos[2]) {
            cameraOffset[0] += TRACK_TRANSFORM;
            cameraOffset[2] += TRACK_TRANSFORM;
        } else if (cameraOffset[0] >= pos[0] && cameraOffset[2] <= pos[2]) {
            cameraOffset[0] -= TRACK_TRANSFORM;
            cameraOffset[2] -= TRACK_TRANSFORM;
        } else if (cameraOffset[0] >= pos[0] && cameraOffset[2] >= pos[2]) {
            cameraOffset[0] += TRACK_TRANSFORM;
            cameraOffset[2] -= TRACK_TRANSFORM;
        }
        float distFinish = cameraOffset[0] * cameraOffset[0] + cameraOffset[2] * cameraOffset[2];
        distHold = sqrt(distHold);
        distFinish = sqrt(distFinish);
        distFinish /= distHold;
        cameraOffset[0] /= distFinish;
        cameraOffset[2] /= distFinish;
    }
    setOffset(cameraOffset);
}

// Returns a normalized ray from the camera to the target
vec3 ComplexCameraObject::getDirRay() {
    auto pos = target_->getPosition();
    auto offset = target_->getPosition(offset_);
    vec3 ray = pos - offset;
    return glm::normalize(ray);
}
