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

extern std::unique_ptr<InputController> inputController;
extern std::unique_ptr<AnimationController> animationController;
extern std::unique_ptr<PhysicsController> physicsController;
#define PI 3.14159265
/// @todo - Use event based input handling; DO NOT REFACTOR THIS FILE
vector<float> cameraDistance(vec3 offset);

/*
 (void) rotateShape takes a (void *) gameInfoStruct that should be of type
 (struct gameInfo *), and a (void *) target that should be of type
 (GameObject *). The rotate shape function allows the user to control the camera
 in the current game scene, as well as a target GameObject. This function should
 run concurrently to the mainLoop that renders the game scene.

 (void) rotateShape does not return a value.
*/
void rotateShape(void *gameInfoStruct, void *target) {
    int mouseX, mouseY, numJoySticks = SDL_NumJoysticks();
    /// @todo Refactor and remove reinterpret_casts if re-using this code
    gameInfo *currentGameInfo = reinterpret_cast<gameInfo *>(gameInfoStruct);
    GameInstance *currentGame = currentGameInfo->currentGame;
    GameObject *character = reinterpret_cast<GameObject *>(target);  // GameObject to rotate
    float rotateSpeed = 1.0f, currentLuminance = 1.0f;
    vec3 angles = vec3(0), pos = vec3(0);
    float fallspeed = 0;
    bool uPressed = false;
    bool sixPressed = false;
    bool lPressed = false;
    bool delPressed = false;
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
    auto tpsCamera = currentGame->getCamera<TPSCameraObject>("mainCamera");
    tpsCamera->setHeadless(true);
    while (!(*currentGameInfo->isDone)) {
        // Calculate the X-Z angle between the camera and target
        // Assume that the target is the origin
        auto charPos = character->getPosition();
        auto cameraPos = currentGameInfo->gameCamera->getOffset();
        float multiplier = 1.0f;
        float speed = 0.3f;
        // y over x
        float angle = angleOfPoint(cameraPos, charPos);
        SDL_GetRelativeMouseState(&mouseX, &mouseY);
        if (hasActiveController) {
            controllerLeftStateY = SDL_GameControllerGetAxis(gameController1,
                SDL_CONTROLLER_AXIS_LEFTY);
            controllerLeftStateX = SDL_GameControllerGetAxis(gameController1,
                SDL_CONTROLLER_AXIS_LEFTX);
        }
        // SDL_WarpMouseInWindow(currentGame->getWindow(), currentGame->getWidth() / 2, currentGame->getHeight() / 2);
        usleep(9000);
        if (inputController->getKeystateRaw()[SDL_SCANCODE_F]) {
            angles[0] -= rotateSpeed;
        }
        if (inputController->getKeystateRaw()[SDL_SCANCODE_R]) {
            angles[0] += rotateSpeed;
        }
        if (inputController->getKeystateRaw()[SDL_SCANCODE_G]) {
            angles[1] -= rotateSpeed;
        }
        if (inputController->getKeystateRaw()[SDL_SCANCODE_T]) {
            angles[1] += rotateSpeed;
        }
        if (inputController->getKeystateRaw()[SDL_SCANCODE_H]) {
            angles[2] -= rotateSpeed;
        }
        if (inputController->getKeystateRaw()[SDL_SCANCODE_Y]) {
            angles[2] += rotateSpeed;
        }
        if (inputController->pollInput(GameInput::SOUTH) || controllerLeftStateY < -JOYSTICK_DEAD_ZONE) {
            angles[1] = -1.0f * angle - 90.0f;
            float xSpeed = sin((angles[1]) * (PI / 180)) * speed;
            float ySpeed = cos((angles[1]) * (PI / 180)) * speed;

            if (controllerLeftStateY < -JOYSTICK_DEAD_ZONE) {
                multiplier = (static_cast<float>(controllerLeftStateY)) / INT16_MAX;
            }
            pos[0] += (xSpeed / 300.0f) * multiplier;
            pos[2] += (ySpeed / 300.0f) * multiplier;
        }
        if (inputController->pollInput(GameInput::NORTH) || controllerLeftStateY > JOYSTICK_DEAD_ZONE) {
            angles[1] = -1.0f * angle + 90.0f;
            float xSpeed = sin(angles[1] * (PI / 180)) * speed;
            float ySpeed = cos(angles[1] * (PI / 180)) * speed;
            if (controllerLeftStateY > JOYSTICK_DEAD_ZONE) {
                multiplier = (static_cast<float>(controllerLeftStateY * -1)) / INT16_MAX;
            }
            pos[0] += (xSpeed / 300.0f) * multiplier;
            pos[2] += (ySpeed / 300.0f) * multiplier;
        }
        if (inputController->pollInput(GameInput::A) && pos[1] == 0) {
            fallspeed = -0.003f;
            // pos[1] += 0.05f;
        }
        if (inputController->getKeystateRaw()[SDL_SCANCODE_E]) {
            fallspeed = 0;
            pos[1] += speed;
        }
        if (inputController->getKeystateRaw()[SDL_SCANCODE_Q]) {
            pos[1] -= speed;
        }
        if (inputController->pollInput(GameInput::WEST) || controllerLeftStateX > JOYSTICK_DEAD_ZONE) {
            angles[1] = -1.0f * angle + 180.0f;
            float xSpeed = sin((angles[1]) * (PI / 180)) * speed;
            float ySpeed = cos((angles[1]) * (PI / 180)) * speed;

            if (controllerLeftStateX > JOYSTICK_DEAD_ZONE) {
                multiplier = (static_cast<float>(controllerLeftStateX * -1)) / INT16_MAX;
            }
            pos[0] += (xSpeed / 300.0f) * multiplier;
            pos[2] += (ySpeed / 300.0f) * multiplier;
        }
        if (inputController->pollInput(GameInput::EAST) || controllerLeftStateX < -JOYSTICK_DEAD_ZONE) {
            angles[1] = -1.0f * angle;
            float xSpeed = sin((angles[1]) * (PI / 180)) * speed;
            float ySpeed = cos((angles[1]) * (PI / 180)) * speed;

            if (controllerLeftStateX < -JOYSTICK_DEAD_ZONE) {
                multiplier = static_cast<float>(controllerLeftStateX) / INT16_MAX;
            }
            pos[0] += (xSpeed / 300.0f) * multiplier;
            pos[2] += (ySpeed / 300.0f) * multiplier;
        }
        if (inputController->getKeystateRaw()[SDL_SCANCODE_C]) {
            currentLuminance += 0.01f;
        }
        if (inputController->getKeystateRaw()[SDL_SCANCODE_V]) {
            currentLuminance -= 0.01f;
        }
        if (inputController->getKeystateRaw()[SDL_SCANCODE_P]) {
            currentGame->changeWindowMode(SDL_WINDOW_FULLSCREEN_DESKTOP);
        } else if (inputController->getKeystateRaw()[SDL_SCANCODE_O]) {
            currentGame->changeWindowMode(SDL_WINDOW_FULLSCREEN);
        } else if (inputController->getKeystateRaw()[SDL_SCANCODE_I]) {
            currentGame->changeWindowMode(0);
        }
        if (inputController->getKeystateRaw()[SDL_SCANCODE_9]) {
            // Rotate on X+ axis
            auto currentRotation = currentGameInfo->gameCamera->getRotation();
            currentRotation[0] += 1.0f;
            currentGameInfo->gameCamera->setRotation(currentRotation);
        } else if (inputController->getKeystateRaw()[SDL_SCANCODE_0]) {
            // Rotate on X- axis
            auto currentRotation = currentGameInfo->gameCamera->getRotation();
            currentRotation[0] -= 1.0f;
            currentGameInfo->gameCamera->setRotation(currentRotation);
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

        if (inputController->getKeystateRaw()[SDL_SCANCODE_U] && !uPressed) {
            uPressed = true;
            if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
                SDL_SetRelativeMouseMode(SDL_TRUE);
            } else {
                SDL_SetRelativeMouseMode(SDL_FALSE);
            }
        } else if (!inputController->getKeystateRaw()[SDL_SCANCODE_U] && uPressed) {
            uPressed = false;
        }
        if (inputController->getKeystateRaw()[SDL_SCANCODE_6] && !sixPressed) {
            sixPressed = true;
            if (currentGame->getActiveScene().get()->getSceneName().compare("3d-demo-scene") == 0) {
                currentGame->setActiveScene("alternate-3d-scene");
                auto altPlayer = currentGame->getSceneObject("alt");
                currentGameInfo->gameCamera->setTarget(altPlayer);
            } else {
                currentGame->setActiveScene("3d-demo-scene");
                auto altPlayer = currentGame->getSceneObject("player");
                currentGameInfo->gameCamera->setTarget(altPlayer);
            }
        } else if (!inputController->getKeystateRaw()[SDL_SCANCODE_6] && sixPressed) {
            sixPressed = false;
        }
        if (inputController->getKeystateRaw()[SDL_SCANCODE_L] && !lPressed) {
            lPressed = true;
            auto enableStatus = ColliderObject::getDrawCollider();
            ColliderObject::setDrawCollider(!enableStatus);
        } else if (!inputController->getKeystateRaw()[SDL_SCANCODE_L] && lPressed) {
            lPressed = false;
        }
        if (inputController->getKeystateRaw()[SDL_SCANCODE_BACKSPACE] && !delPressed) {
            delPressed = true;
            static int bulletCount;
            // Instantiate a bullet and shoot it
            currentGame->protectedGfxRequest([currentGame, character, angle] () {
                auto imp = ModelImport::createPolygonFromFile("src/resources/models/bullet.obj");
                auto bullet = currentGame->createGameObject(imp, character->getPosition(), vec3(angle, 0, angle - 180.0f), 0.005f,
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
                auto delcb = [bulletName, currentGame] () {
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
                auto anglex = std::cos(angles.y * (PI/180.0) - (PI/2.0));
                auto anglez = std::sin(angles.y * (PI/180.0) + (PI/2.0));
                auto charAngle = character->getRotation();
                float magnitude = 0.01f;
                // angles.y is the rotation of the character on some axis??
                printf("Detected rot %f %f %f\n", charAngle.x, charAngle.y, charAngle.z);
                physicsController->setVelocity(bulletName, vec3(anglex * magnitude, 0.0f, anglez * magnitude));
            }
        } else if (!inputController->getKeystateRaw()[SDL_SCANCODE_BACKSPACE] && delPressed) {
            delPressed = false;
        }
        // Set character rotation based on joysticks
        // Left rotation
        if (abs(controllerLeftStateX) > JOYSTICK_DEAD_ZONE || abs(controllerLeftStateY) > JOYSTICK_DEAD_ZONE) {
            angles[1] = angleOfPoint(
                vec3(0.0f, 0.0f, 0.0f),
                vec3(static_cast<float>(controllerLeftStateY),
                    0.0f,
                    static_cast<float>(controllerLeftStateX))) - 90.0f - angle;
        }
        fallspeed = basicPhysics(&pos[1], fallspeed);
        // cout << "CO - X: " << cameraOffset[0] << ", Y: " << cameraOffset[1]
        //    << ", Z: " << cameraOffset[2] << "\n";
        // cout << "dx: " << mouseX << ", dy: " << mouseY << "\n";
        currentGame->protectedGfxRequest([&] () {
            currentGame->setLuminance(currentLuminance);
            character->setRotation(angles);
            character->setPosition(pos);
            tpsCamera->updateInput();
        });
    }
    SDL_GameControllerClose(gameController1);
    return;
} //NOLINT - refactor required

/*
 (vector<float>) cameraDistance takes a 3D vector containing the offset of the
 camera from the object and calculates the distance between the two with
 respect to the y-z plane, and x-y plane. Returns a 2D vector where the first
 element is the y-z distance, and the second element is the x-y distance.
*/
vector<float> cameraDistance(vec3 offset) {
    vector<float> distance(2);
    distance[0] = offset[1] * offset[1] + offset[2] * offset[2];
    distance[1] = offset[1] * offset[1] + offset[0] * offset[0];
    return distance;
}

float convertNegToDeg(float degree) {
    return degree >= 0.0f ? degree : degree + 360.0f;
}

// Calculates the angle between two points in degrees
float angleOfPoint(vec3 p1, vec3 p2) {
    auto diffPoint = p2 - p1;
    float angle = atan2(diffPoint[2], diffPoint[0]) * (180.0f / PI);
    return convertNegToDeg(angle);
}
