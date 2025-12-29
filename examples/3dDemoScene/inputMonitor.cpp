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
#include "glm/trigonometric.hpp"
#include "physics.hpp"
#include <TPSCameraObject.hpp>
#include <SDL_scancode.h>
#include <iostream>
#include <vector>
#include <string>
#include <cstdio>
#include <cmath>
#include <ColliderObject.hpp>
#include <ModelImport.hpp>
#include <AnimationController.hpp>
// Analog joystick dead zone
extern const int JOYSTICK_DEAD_ZONE;
extern GameInstance *currentGame;

#define UPDATE_TRAVEL_VEL travelVel += (inputRay * speed * multiplier)
#define UPDATE_CHAR_ANGLE(shift) charAngle[1] = glm::degrees(glm::atan(ray.x, ray.z)) + shift

extern std::unique_ptr<InputController> inputController;
extern std::unique_ptr<AnimationController> animationController;
extern std::unique_ptr<PhysicsController> physicsController;
#define PI 3.14159265
vector<float> cameraDistance(vec3 offset);

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
    float rotateSpeed = 1.0f, currentLuminance = 1.0f;
    auto fpsMode = false;
    /** Input map code example from BOTHWORLDS */
    vector<int> monitoredInput = {
        SDL_SCANCODE_6,
        SDL_SCANCODE_L,
        SDL_SCANCODE_U,
        SDL_SCANCODE_P,
        SDL_SCANCODE_O,
        SDL_SCANCODE_I,
        SDL_SCANCODE_T,
        SDL_SCANCODE_BACKSPACE
    };

    map<int, bool> debounceMap;
    map<int, bool> inputMap;
    auto initDebounceMap = [&debounceMap, &monitoredInput] () {
        for (auto i : monitoredInput) {
            debounceMap[i] = true;
        }
    };
    auto populateInputMap = [&debounceMap, &monitoredInput] (map<int, bool> &inputMap) {
        for (auto input : monitoredInput) {
            auto state = inputController->getKeystateRaw()[input];
            // Set input map to true if debounce = false
            inputMap[input] = !debounceMap[input] && state;
            // Set debounce state
            debounceMap[input] = state;
        }
    };
    initDebounceMap();

    cout << rotateSpeed;
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
    // Don't update via SceneObject::update - instead update manually to avoid jitters (overkill for polish)
    tpsCamera->setHeadless(true);
    fpsCamera->setHeadless(true);
    currentGame->setActiveCamera("tpsCamera");
    while (!currentGame->isShutDown()) {
        populateInputMap(inputMap);
        auto activeCamera = currentGame->getActiveCamera<TPSCameraObject>();
        // Calculate the X-Z angle between the camera and target
        // Assume that the target is the origin
        auto charAngle = character->getRotation();
        vec3 travelVel = vec3(0);
        auto ray = activeCamera->getDirRay();
        // Remove the Y axis from the ray
        ray.y = 0.0f;
        float multiplier = 1.0f;
        cout << multiplier<<endl;
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
            UPDATE_CHAR_ANGLE(90.0f);
            UPDATE_TRAVEL_VEL;
        }
        if (inputController->pollInput(GameInput::EAST) || controllerLeftStateX < -JOYSTICK_DEAD_ZONE) {
            vec3 inputRay(-ray.z, 0, ray.x);
            if (controllerLeftStateX < -JOYSTICK_DEAD_ZONE) {
                multiplier = static_cast<float>(controllerLeftStateX) / INT16_MAX;
            }
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

        if (inputMap[SDL_SCANCODE_U]) {
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
        if (inputMap[SDL_SCANCODE_T]) {
            // Switch the active camera
            if (activeCamera->objectName() == "fpsCamera") {
                currentGame->setActiveCamera("tpsCamera");
                fpsMode = false;
            } else {
                currentGame->setActiveCamera("fpsCamera");
                fpsMode = true;
            }
        }
        if (inputMap[SDL_SCANCODE_BACKSPACE]) {
            static int bulletCount;
            // Instantiate a bullet and shoot it
            currentGame->protectedGfxRequest([character] () {
                auto imp = ModelImport::createPolygonFromFile("src/resources/models/bullet.obj");
                // use the same angle as the direction vector
                auto bullet = currentGame->createGameObject(imp, character->getPosition(), vec3(0.0f), 0.005f,
                    string("bullet") + std::to_string(bulletCount++));
                bullet->createCollider();
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
        // Left rotation
        if (abs(controllerLeftStateX) > JOYSTICK_DEAD_ZONE || abs(controllerLeftStateY) > JOYSTICK_DEAD_ZONE) {
            // angles[1] = angleOfPoint(
            //     vec3(0.0f, 0.0f, 0.0f),
            //     vec3(static_cast<float>(controllerLeftStateY),
            //         0.0f,
            //         static_cast<float>(controllerLeftStateX))) - 90.0f - angle;
        }
        physicsController->setVelocity("player", travelVel);
        currentGame->protectedGfxRequest([&] () {
            currentGame->setLuminance(currentLuminance);
            character->setRotation(charAngle);
            activeCamera->updateInput();
        });
    }
    SDL_GameControllerClose(gameController1);
    return;
} //NOLINT - refactor required
