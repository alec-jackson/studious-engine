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
#define TRACK_TRANSFORM TRACKING_SPEED * modifier * deltaTime

ComplexCameraObject::ComplexCameraObject(SceneObject *target, vec3 offset, float cameraAngle, float aspectRatio,
    float nearClipping, float farClipping, ObjectType type, string objectName, GfxController *gfxController) :
    CameraObject(target, offset, cameraAngle, aspectRatio, nearClipping, farClipping, type, objectName, gfxController) {
    init();
}

void ComplexCameraObject::update() {
    if (!headless_) updateInput();
    CameraObject::update();
}

void ComplexCameraObject::init() {
    // Replace with InputController code later...
    int numJoySticks = SDL_NumJoysticks();
    cameraOffset = getOffset(), angles = vec3(0),
        pos = vec3(0);
    // SDL_GetRelativeMouseMode()
    gameController1 = NULL;
    hasActiveController = false;
    if (numJoySticks < 1) {
        cout << "No joysticks connected, continuing without joysticks\n";
    } else {
        for (int i = 0; i < numJoySticks; i++) {
            if (SDL_IsGameController(i)) {
                gameController1 = SDL_GameControllerOpen(i);
                if (gameController1 != 0) {
                    hasActiveController = true;
                    break;
                }
            }
        }
    }
}

void ComplexCameraObject::updateInput() {
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
    // Do nothing in relative mouse mode
    if (!SDL_GetRelativeMouseMode()) {
        ignoreFirstUpdate_ = true;
        return;
    }
    // Calculate the X-Z angle between the camera and target
    // Assume that the target is the origin
    int mouseX, mouseY;
    Sint16 controllerRightStateY = 0;
    Sint16 controllerRightStateX = 0;
    // Allow the mouse to capture - prevents jitters when attaching to camera
    if (ignoreFirstUpdate_) {
        SDL_GetRelativeMouseState(&mouseX, &mouseY);
        ignoreFirstUpdate_ = false;
        return;
    }
    cameraOffset = offset_;
    // y over x
    SDL_GetRelativeMouseState(&mouseX, &mouseY);
    if (hasActiveController) {
        controllerRightStateY = SDL_GameControllerGetAxis(gameController1,
            SDL_CONTROLLER_AXIS_RIGHTY);
        controllerRightStateX = SDL_GameControllerGetAxis(gameController1,
            SDL_CONTROLLER_AXIS_RIGHTX);
    }
    if ((mouseY < 0) || controllerRightStateY < -JOYSTICK_DEAD_ZONE) {
        float modifier = 1.0f;
        if (mouseY < 0) {
            modifier = (mouseY / 5.0f) * -1;
        } else if (controllerRightStateY < -JOYSTICK_DEAD_ZONE) {
            modifier = (static_cast<float>(controllerRightStateY * -1)) / INT16_MAX;
        }
        INVERT_MODIFIER(invertY);
        vector<float> distHold = cameraDistance(cameraOffset);
        cameraOffset[1] -= TRACK_TRANSFORM;
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
    if ((mouseY > 0) || controllerRightStateY > JOYSTICK_DEAD_ZONE) {
        float modifier = 1.0f;
        if (mouseY > 0) {
            modifier = mouseY / 5.0f;
        } else if (controllerRightStateY > JOYSTICK_DEAD_ZONE) {
            modifier = static_cast<float>(controllerRightStateY) / INT16_MAX;
        }
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
    if ((mouseX < 0) || controllerRightStateX < -JOYSTICK_DEAD_ZONE) {
        // Rotate the camera about the y axis
        float distHold = cameraOffset[0] * cameraOffset[0] + cameraOffset[2] * cameraOffset[2];
        float modifier = 1.0f;
        if (mouseX < 0) {
            modifier = (mouseX * -1.0f) / 5.0f;
        } else if (controllerRightStateX < -JOYSTICK_DEAD_ZONE) {
            modifier = (static_cast<float>(controllerRightStateX * -1)) / INT16_MAX;
        }
        INVERT_MODIFIER(invertX);
        if (cameraOffset[0] <= pos[0] && cameraOffset[2] <= pos[2]) {
            cameraOffset[0] += TRACK_TRANSFORM;
            cameraOffset[2] -= TRACK_TRANSFORM;
        } else if (cameraOffset[0] <= pos[0] && cameraOffset[2] >= pos[2]) {
            cameraOffset[0] -= TRACK_TRANSFORM;
            cameraOffset[2] -= TRACK_TRANSFORM;
        } else if (cameraOffset[0] >= pos[0] && cameraOffset[2] <= pos[2]) {
            cameraOffset[0] += TRACK_TRANSFORM;
            cameraOffset[2] += TRACK_TRANSFORM;
        } else if (cameraOffset[0] >= pos[0] && cameraOffset[2] >= pos[2]) {
            cameraOffset[0] -= TRACK_TRANSFORM;
            cameraOffset[2] += TRACK_TRANSFORM;
        }
        float distFinish = cameraOffset[0] * cameraOffset[0] + cameraOffset[2] * cameraOffset[2];
        distHold = sqrt(distHold);
        distFinish = sqrt(distFinish);
        distFinish /= distHold;
        cameraOffset[0] /= distFinish;
        cameraOffset[2] /= distFinish;
    }
    if ((mouseX > 0) || controllerRightStateX > JOYSTICK_DEAD_ZONE) {
        float distHold = cameraOffset[0] * cameraOffset[0] + cameraOffset[2] * cameraOffset[2];
        float modifier = 1.0f;
        if (mouseX > 0) {
            modifier = mouseX / 5.0f;
        } else if (controllerRightStateX > JOYSTICK_DEAD_ZONE) {
            modifier = static_cast<float>(controllerRightStateX) / INT16_MAX;
        }
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
