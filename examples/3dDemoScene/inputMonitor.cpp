/**
 * @file inputMonitor.cpp
 * @author Alec Jackson
 * @brief Contains functions for manipulating GameObjects given some input. Supports keyboard, mouse and controller
 * @version 0.1
 * @date 2023-07-28
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "inputMonitor.hpp"
#include <ComplexCameraObject.hpp>
#include <InputController.hpp>
#include <glm/trigonometric.hpp>
#include <physics.hpp>
#include <TPSCameraObject.hpp>
#include <SDL_scancode.h>
#include <glm/ext/quaternion_geometric.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <cstdio>
#include <cmath>
#include <ColliderObject.hpp>
#include <ModelImport.hpp>
#include <AnimationController.hpp>
// Analog joystick dead zone
extern GameInstance *currentGame;

#define UPDATE_TRAVEL_VEL travelVel += (inputRay * speed * multiplier)
#define UPDATE_CHAR_ANGLE(shift) charAngle[1] = glm::degrees(glm::atan(ray.x, ray.z)) + shift
#define INVERT_MULT_FPS multiplier *= fpsMode ? -1.0f : 1.0f

extern std::unique_ptr<InputController> inputController;
extern std::unique_ptr<AnimationController> animationController;
extern std::unique_ptr<PhysicsController> physicsController;
#define PI 3.14159265

void updateAttachStatus();

/*
 (void) rotateShape takes a (void *) gameInfoStruct that should be of type
 (struct gameInfo *), and a (void *) target that should be of type
 (GameObject *). The rotate shape function allows the user to control the camera
 in the current game scene, as well as a target GameObject. This function should
 run concurrently to the mainLoop that renders the game scene.

 (void) rotateShape does not return a value.
*/
void rotateShape(void *target) {
    int numJoySticks = SDL_NumJoysticks();
    GameObject *character = reinterpret_cast<GameObject *>(target);  // GameObject to rotate
    float currentLuminance = 1.0f;
    auto fpsMode = false;
    /** Input map code example from BOTHWORLDS */
    vector<int> monitoredInput = {
        SDL_SCANCODE_6,
        SDL_SCANCODE_L,
        SDL_SCANCODE_P,
        SDL_SCANCODE_O,
        SDL_SCANCODE_I
    };

    vector<GameInput> monitoredGInput = {
        GameInput::START, // Attach camera
        GameInput::B,  // fire bullet
        GameInput::Y,  // switch FPS -> TPS
        GameInput::A   // Jump
    };

    map<int, bool> debounceMap;
    map<int, bool> inputMap;
    map<GameInput, bool> debounceGIMap;
    map<GameInput, bool> inputGIMap;
    auto initDebounceMaps = [&debounceMap, &debounceGIMap, &monitoredInput, &monitoredGInput] () {
        for (auto i : monitoredInput) {
            debounceMap[i] = true;
        }
        for (auto i : monitoredGInput) {
            debounceGIMap[i] = true;
        }
    };
    auto populateInputMaps = [&debounceMap, &debounceGIMap, &monitoredInput, &monitoredGInput] (map<int, bool> &inputMap, map<GameInput, bool> &inputGIMap) {
        for (auto input : monitoredInput) {
            auto state = inputController->getKeystateRaw()[input];
            // Set input map to true if debounce = false
            inputMap[input] = !debounceMap[input] && state;
            // Set debounce state
            debounceMap[input] = state;
        }
        for (auto input : monitoredGInput) {
            auto state = inputController->pollInput(input);
            inputGIMap[input] = !debounceGIMap[input] && state;
            debounceGIMap[input] = state;
        }
    };
    initDebounceMaps();

    // bool delPressed = false;
    SDL_GameController *gameController1 = NULL;
    bool hasActiveController = false;
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
    Sint16 controllerLeftStateY = 0;
    Sint16 controllerLeftStateX = 0;
    auto tpsCamera = currentGame->getCamera<TPSCameraObject>("tpsCamera");
    auto fpsCamera = currentGame->getCamera<FPSCameraObject>("fpsCamera");
    currentGame->setActiveCamera("tpsCamera");
    while (!currentGame->isShutDown()) {
        updateAttachStatus();
        populateInputMaps(inputMap, inputGIMap);
        auto activeCamera = currentGame->getActiveCamera<ComplexCameraObject>();
        // Calculate the X-Z angle between the camera and target
        // Assume that the target is the origin
        auto charAngle = character->getRotation();
        vec3 travelVel = vec3(0);
        auto ray = activeCamera->getDirRay();
        // Remove the Y axis from the ray
        ray.y = 0.0f;
        float multiplier = 1.0f;
        float speed = 4.0f;
        if (hasActiveController) {
            controllerLeftStateY = SDL_GameControllerGetAxis(gameController1,
                SDL_CONTROLLER_AXIS_LEFTY);
            controllerLeftStateX = SDL_GameControllerGetAxis(gameController1,
                SDL_CONTROLLER_AXIS_LEFTX);
        }
        // SDL_WarpMouseInWindow(currentGame->getWindow(), currentGame->getWidth() / 2, currentGame->getHeight() / 2);
        usleep(9000);
        if (inputController->pollInput(GameInput::SOUTH) || controllerLeftStateY < -JOYSTICK_DEAD_ZONE) {
            // Reverse the ray
            auto inputRay = fpsMode ? ray : -1.0f * ray;
            if (controllerLeftStateY < -JOYSTICK_DEAD_ZONE) {
                multiplier = (static_cast<float>(controllerLeftStateY)) / INT16_MAX;
            }
            // Rotate 180 degrees to face back
            UPDATE_CHAR_ANGLE((fpsMode ? 0.0f : 180.0f));
            UPDATE_TRAVEL_VEL;
        }
        if (inputController->pollInput(GameInput::NORTH) || controllerLeftStateY > JOYSTICK_DEAD_ZONE) {
            auto inputRay = fpsMode ? -1.0f * ray : ray;
            if (controllerLeftStateY > JOYSTICK_DEAD_ZONE) {
                multiplier = (static_cast<float>(controllerLeftStateY * -1)) / INT16_MAX;
            }
            UPDATE_CHAR_ANGLE((fpsMode ? 180.0f : 0.0f));
            // For north, just follow the ray
            UPDATE_TRAVEL_VEL;
        }
        if (inputController->pollInput(GameInput::WEST) || controllerLeftStateX > JOYSTICK_DEAD_ZONE) {
            // Fix angle shift here...
            // Swap XZ points for input ray
            vec3 inputRay(ray.z, 0, -ray.x);

            if (controllerLeftStateX > JOYSTICK_DEAD_ZONE) {
                multiplier = (static_cast<float>(controllerLeftStateX * -1)) / INT16_MAX;
            }
            INVERT_MULT_FPS;
            UPDATE_CHAR_ANGLE(90.0f);
            UPDATE_TRAVEL_VEL;
        }
        if (inputController->pollInput(GameInput::EAST) || controllerLeftStateX < -JOYSTICK_DEAD_ZONE) {
            vec3 inputRay(-ray.z, 0, ray.x);
            if (controllerLeftStateX < -JOYSTICK_DEAD_ZONE) {
                multiplier = static_cast<float>(controllerLeftStateX) / INT16_MAX;
            }
            INVERT_MULT_FPS;
            UPDATE_CHAR_ANGLE(270.0f);
            UPDATE_TRAVEL_VEL;
        }
        if (inputMap[SDL_SCANCODE_P]) {
            currentGame->changeWindowMode(SDL_WINDOW_FULLSCREEN_DESKTOP);
        } else if (inputMap[SDL_SCANCODE_O]) {
            currentGame->changeWindowMode(SDL_WINDOW_FULLSCREEN);
        } else if (inputMap[SDL_SCANCODE_I]) {
            currentGame->changeWindowMode(0);
        }

        // Move the directional light
        if (inputController->getKeystateRaw()[SDL_SCANCODE_7]) {
            // Shift light origin diagonal pos
            auto dirLight = currentGame->getDirectionalLight();
            dirLight += vec3(1.0f);
            currentGame->setDirectionalLight(dirLight);
        } else if (inputController->getKeystateRaw()[SDL_SCANCODE_8]) {
            // Shift light origin diagonal neg
            auto dirLight = currentGame->getDirectionalLight();
            dirLight -= vec3(1.0f);
            currentGame->setDirectionalLight(dirLight);
        }

        if (inputGIMap[GameInput::START]) {
            if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
                SDL_SetRelativeMouseMode(SDL_TRUE);
            } else {
                SDL_SetRelativeMouseMode(SDL_FALSE);
            }
        }
        if (inputMap[SDL_SCANCODE_6]) {
            if (currentGame->getActiveScene().get()->getSceneName().compare("3d-demo-scene") == 0) {
                currentGame->setActiveScene("alternate-3d-scene");
                auto altPlayer = currentGame->getSceneObject("alt");
                activeCamera->setTarget(altPlayer);
            } else {
                currentGame->setActiveScene("3d-demo-scene");
                auto altPlayer = currentGame->getSceneObject("player");
                activeCamera->setTarget(altPlayer);
            }
        }
        if (inputMap[SDL_SCANCODE_L]) {
            auto enableStatus = ColliderObject::getDrawCollider();
            ColliderObject::setDrawCollider(!enableStatus);
        }
        if (inputGIMap[GameInput::Y]) {
            // Switch the active camera
            if (activeCamera->objectName() == "fpsCamera") {
                // Sync offset between cameras
                tpsCamera->setOffset(fpsCamera->getOffset() * -1.0f);
                currentGame->setActiveCamera("tpsCamera");
                fpsMode = false;
                character->setVisible(true);
            } else {
                fpsCamera->setOffset(tpsCamera->getOffset() * -1.0f);
                currentGame->setActiveCamera("fpsCamera");
                fpsMode = true;
                // Hide drac in FPS mode
                character->setVisible(false);
            }
        }
        static bool stopFlush;
        auto doFlush = true;
        if (inputGIMap[GameInput::A]) {
            // Apply instant force to the object
            physicsController->setVelocity("player", vec3(0, 10.0f, 0));
            //travelVel += vec3(0, 50.0f, 0);
            doFlush = false;
            stopFlush = true;
        }
        if (inputGIMap[GameInput::B]) {
            static int bulletCount;
            // Instantiate a bullet and shoot it
            currentGame->protectedGfxRequest([character] () {
                auto imp = ModelImport::createPolygonFromFile("src/resources/models/bullet.obj");
                // use the same angle as the direction vector
                auto bullet = currentGame->createGameObject(imp, character->getPosition(), vec3(0.0f), 1.0f,
                    string("bullet") + std::to_string(bulletCount++));
                bullet->createCollider("bullet");
            });
            auto bulletName = string("bullet") + std::to_string(bulletCount - 1);
            auto bulletObj = currentGame->getSceneObject(bulletName);
            auto expireTime = 900.0f;
            if (nullptr == bulletObj) {
                fprintf(stderr, "rotateShape: Failed to create bullet object!");
            } else {
                bulletObj->setRenderPriority(RENDER_PRIOR_LOW - 2);
                currentGame->getActiveScene()->refresh();
                // Decay the bullet after 5 seconds
                auto delcb = [bulletName] () {
                    currentGame->removeSceneObject(bulletName);
                };
                auto bkf = AnimationController::createKeyFrameCb(UPDATE_NONE, delcb, expireTime);
                animationController->addKeyFrame(bulletObj, bkf);

                PhysicsParams params = {
                    .isKinematic = true,
                    .obeyGravity = false,
                    .elasticity = 0.0f,
                    .mass = 1.0f
                };
                physicsController->addSceneObject(bulletObj, params);
                // Convert angles[1] to a direction????
                //auto anglex = std::cos(angles.y * (PI/180.0) - (PI/2.0));
                //auto anglez = std::sin(angles.y * (PI/180.0) + (PI/2.0));
                auto charAngle = character->getRotation();
                float magnitude = 0.01f;
                // angles.y is the rotation of the character on some axis??
                printf("Detected rot %f %f %f\n", charAngle.x, charAngle.y, charAngle.z);

                physicsController->setVelocity(bulletName, vec3(ray.x * magnitude, 0.0f, ray.z * magnitude));
            }
        }
        // Set character rotation based on joysticks
        if (abs(controllerLeftStateX) > JOYSTICK_DEAD_ZONE || abs(controllerLeftStateY) > JOYSTICK_DEAD_ZONE) {
            auto prod = (float)controllerLeftStateY / (float)controllerLeftStateX;
            auto angle = glm::degrees(atan(prod));
            angle += controllerLeftStateX > 0.0f ? 90.0f : 270.0f;
            UPDATE_CHAR_ANGLE((fpsMode ? -angle + 180.0f : -angle));
        }
        if (!stopFlush)
            physicsController->setVelocity("player", travelVel, doFlush);
        currentGame->protectedGfxRequest([&] () {
            currentGame->setLuminance(currentLuminance);
            character->setRotation(charAngle);
        });
    }
    SDL_GameControllerClose(gameController1);
    return;
} //NOLINT - refactor required

void updateAttachStatus() {
    auto attached = SDL_GetRelativeMouseMode() == SDL_TRUE ? true : false;
    currentGame->protectedGfxRequestAsync([attached] () {
        auto pressUText = currentGame->getSceneObject<TextObject>("pressUText");
        assert(pressUText != nullptr);
        if (attached) {
            pressUText->setMessage("Tab/Start to Focus (attached)");
            pressUText->setColor(vec4(0.0f, 1.0f, 0.0f, 1.0f));
        } else {
            pressUText->setMessage("Tab/Start to Focus (detached)");
            pressUText->setColor(vec4(1.0f));
        }
    });
}
