#include <TPSCameraObject.hpp>
#include <InputController.hpp>
#include <AnimationController.hpp>
#include <physics.hpp>
#include <memory>
#include <thread>

extern std::unique_ptr<InputController> inputController;
extern std::unique_ptr<AnimationController> animationController;
extern std::unique_ptr<PhysicsController> physicsController;

TPSCameraObject::TPSCameraObject(SceneObject *target, vec3 offset, float cameraAngle, float aspectRatio,
    float nearClipping, float farClipping, ObjectType type, string objectName, GfxController *gfxController) :
    CameraObject(target, offset, cameraAngle, aspectRatio, nearClipping, farClipping, type, objectName, gfxController) {
        // Do some extra stuff in addition to normal camera work
        // Spin up a thread for input monitoring?
    init();
}

void TPSCameraObject::update() {
    if (!headless_) updateInput();
    CameraObject::update();
}

void TPSCameraObject::init() {
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

void TPSCameraObject::updateInput() {
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
    if (!SDL_GetRelativeMouseMode()) return;
    // Calculate the X-Z angle between the camera and target
    // Assume that the target is the origin
    int mouseX, mouseY;
    Sint16 controllerRightStateY = 0;
    Sint16 controllerRightStateX = 0;
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
        vector<float> distHold = cameraDistance(cameraOffset);
        cameraOffset[1] -= TRACKING_SPEED * modifier;
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
        vector<float> distHold = cameraDistance(cameraOffset);
        cameraOffset[1] += TRACKING_SPEED * modifier;
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
        float multiplier = 1.0f;
        if (mouseX < 0) {
            multiplier = (mouseX * -1.0f) / 5.0f;
        } else if (controllerRightStateX < -JOYSTICK_DEAD_ZONE) {
            multiplier = (static_cast<float>(controllerRightStateX * -1)) / INT16_MAX;
        }
        if (cameraOffset[0] <= pos[0] && cameraOffset[2] <= pos[2]) {
            cameraOffset[0] += TRACKING_SPEED * multiplier;
            cameraOffset[2] -= TRACKING_SPEED * multiplier;
        } else if (cameraOffset[0] <= pos[0] && cameraOffset[2] >= pos[2]) {
            cameraOffset[0] -= TRACKING_SPEED * multiplier;
            cameraOffset[2] -= TRACKING_SPEED * multiplier;
        } else if (cameraOffset[0] >= pos[0] && cameraOffset[2] <= pos[2]) {
            cameraOffset[0] += TRACKING_SPEED * multiplier;
            cameraOffset[2] += TRACKING_SPEED * multiplier;
        } else if (cameraOffset[0] >= pos[0] && cameraOffset[2] >= pos[2]) {
            cameraOffset[0] -= TRACKING_SPEED * multiplier;
            cameraOffset[2] += TRACKING_SPEED * multiplier;
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
        float multiplier = 1.0f;
        if (mouseX > 0) {
            multiplier = mouseX / 5.0f;
        } else if (controllerRightStateX > JOYSTICK_DEAD_ZONE) {
            multiplier = static_cast<float>(controllerRightStateX) / INT16_MAX;
        }
        if (cameraOffset[0] <= pos[0] && cameraOffset[2] <= pos[2]) {
            cameraOffset[0] -= TRACKING_SPEED * multiplier;
            cameraOffset[2] += TRACKING_SPEED * multiplier;
        } else if (cameraOffset[0] <= pos[0] && cameraOffset[2] >= pos[2]) {
            cameraOffset[0] += TRACKING_SPEED * multiplier;
            cameraOffset[2] += TRACKING_SPEED * multiplier;
        } else if (cameraOffset[0] >= pos[0] && cameraOffset[2] <= pos[2]) {
            cameraOffset[0] -= TRACKING_SPEED * multiplier;
            cameraOffset[2] -= TRACKING_SPEED * multiplier;
        } else if (cameraOffset[0] >= pos[0] && cameraOffset[2] >= pos[2]) {
            cameraOffset[0] += TRACKING_SPEED * multiplier;
            cameraOffset[2] -= TRACKING_SPEED * multiplier;
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
