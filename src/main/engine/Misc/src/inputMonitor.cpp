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

#include <inputMonitor.hpp>
// Analog joystick dead zone
const int JOYSTICK_DEAD_ZONE = 4000;
#define PI 3.14159265
/// @todo - Use event based input handling; DO NOT REFACTOR THIS FILE
vector<double> cameraDistance(vec3 offset);

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
    GLfloat speed = 0.001f, rotateSpeed = 1.0f, scaleSpeed = 0.0002f,
        offsetSpeed = 0.1f, currentScale = character->getScale(),
        currentLuminance = 1.0f;
    vec3 cameraOffset = vec3(7.140022f, 1.349999f, 2.309998f), angles = vec3(0),
        pos = vec3(0);
    float fallspeed = 0;
    bool trackMouse = false;
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
    Sint16 controllerRightStateY = 0;
    Sint16 controllerRightStateX = 0;
    Uint8 buttonCheckA = 0;
    while (!(*currentGameInfo->isDone)) {
        // Calculate the X-Z angle between the camera and target
        // Assume that the target is the origin
        auto charPos = character->getPosition();
        auto cameraPos = currentGameInfo->gameCamera->getOffset();
        auto cameraDiffPos = cameraPos - charPos;
        double multiplier = 0.3f;
        // y over x
        float angle = angleOfPoint(cameraPos, charPos);
        SDL_GetRelativeMouseState(&mouseX, &mouseY);
        if (hasActiveController) {
            controllerLeftStateY = SDL_GameControllerGetAxis(gameController1,
                SDL_CONTROLLER_AXIS_LEFTY);
            controllerLeftStateX = SDL_GameControllerGetAxis(gameController1,
                SDL_CONTROLLER_AXIS_LEFTX);
            controllerRightStateY = SDL_GameControllerGetAxis(gameController1,
                SDL_CONTROLLER_AXIS_RIGHTY);
            controllerRightStateX = SDL_GameControllerGetAxis(gameController1,
                SDL_CONTROLLER_AXIS_RIGHTX);
            buttonCheckA = SDL_GameControllerGetButton(gameController1,
                SDL_CONTROLLER_BUTTON_A);
        }
        // SDL_WarpMouseInWindow(currentGame->getWindow(), currentGame->getWidth() / 2, currentGame->getHeight() / 2);
        usleep(9000);
        // Begin Camera Controls
        if (currentGame->getKeystate()[SDL_SCANCODE_KP_5]) {
            cameraOffset[0] = 5.140022f;  // Set values back to inital values
            cameraOffset[1] = 1.349999f;
            cameraOffset[2] = 2.309998f;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_KP_2] ||
            (mouseY < 0 && trackMouse) || controllerRightStateY > JOYSTICK_DEAD_ZONE) {
            float modifier = 1.0f;
            if (mouseY < 0) {
                modifier = (mouseY / 5.0f) * -1;
            } else if (controllerRightStateY > JOYSTICK_DEAD_ZONE) {
                modifier = static_cast<float>(controllerRightStateY) / 32767;
            }
            vector<double> distHold = cameraDistance(cameraOffset);
            cameraOffset[1] -= offsetSpeed * modifier;
            vector<double> distFinish = cameraDistance(cameraOffset);
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
        if (currentGame->getKeystate()[SDL_SCANCODE_KP_8] ||
            (mouseY > 0 && trackMouse) || controllerRightStateY < -JOYSTICK_DEAD_ZONE) {
            // cameraOffset[1] += offsetSpeed;
            float modifier = 1.0f;
            if (mouseY > 0) {
                modifier = mouseY / 5.0f;
            } else if (controllerRightStateY < -JOYSTICK_DEAD_ZONE) {
                modifier = (static_cast<double>(controllerRightStateY * -1)) / 32767;
            }
            vector<double> distHold = cameraDistance(cameraOffset);
            cameraOffset[1] += offsetSpeed * modifier;
            vector<double> distFinish = cameraDistance(cameraOffset);
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
        if (currentGame->getKeystate()[SDL_SCANCODE_KP_4]) {
            cameraOffset[0] -= offsetSpeed;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_KP_6]) {
            cameraOffset[0] += offsetSpeed;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_KP_7] ||
            (mouseX < 0 && trackMouse) || controllerRightStateX < -JOYSTICK_DEAD_ZONE) {
            // Rotate the camera about the y axis
            double distHold = cameraOffset[0] * cameraOffset[0] + cameraOffset[2] * cameraOffset[2];
            float multiplier = 1.0f;
            if (mouseX < 0) {
                multiplier = (mouseX * -1.0f) / 5.0f;
            } else if (controllerRightStateX < -JOYSTICK_DEAD_ZONE) {
                multiplier = (static_cast<float>(controllerRightStateX * -1)) / 32767;
            }
            if (cameraOffset[0] <= pos[0] && cameraOffset[2] <= pos[2]) {
                cameraOffset[0] += offsetSpeed * multiplier;
                cameraOffset[2] -= offsetSpeed * multiplier;
            } else if (cameraOffset[0] <= pos[0] && cameraOffset[2] >= pos[2]) {
                cameraOffset[0] -= offsetSpeed * multiplier;
                cameraOffset[2] -= offsetSpeed * multiplier;
            } else if (cameraOffset[0] >= pos[0] && cameraOffset[2] <= pos[2]) {
                cameraOffset[0] += offsetSpeed * multiplier;
                cameraOffset[2] += offsetSpeed * multiplier;
            } else if (cameraOffset[0] >= pos[0] && cameraOffset[2] >= pos[2]) {
                cameraOffset[0] -= offsetSpeed * multiplier;
                cameraOffset[2] += offsetSpeed * multiplier;
            }
            double distFinish = cameraOffset[0] * cameraOffset[0] + cameraOffset[2] * cameraOffset[2];
            distHold = sqrt(distHold);
            distFinish = sqrt(distFinish);
            distFinish /= distHold;
            cameraOffset[0] /= distFinish;
            cameraOffset[2] /= distFinish;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_KP_9] ||
            (mouseX > 0 && trackMouse) || controllerRightStateX > JOYSTICK_DEAD_ZONE) {
            double distHold = cameraOffset[0] * cameraOffset[0] + cameraOffset[2] * cameraOffset[2];
            float multiplier = 1.0f;
            if (mouseX > 0) {
                multiplier = mouseX / 5.0f;
            } else if (controllerRightStateX > JOYSTICK_DEAD_ZONE) {
                multiplier = static_cast<float>(controllerRightStateX) / 32767;
            }
            if (cameraOffset[0] <= pos[0] && cameraOffset[2] <= pos[2]) {
                cameraOffset[0] -= offsetSpeed * multiplier;
                cameraOffset[2] += offsetSpeed * multiplier;
            } else if (cameraOffset[0] <= pos[0] && cameraOffset[2] >= pos[2]) {
                cameraOffset[0] += offsetSpeed * multiplier;
                cameraOffset[2] += offsetSpeed * multiplier;
            } else if (cameraOffset[0] >= pos[0] && cameraOffset[2] <= pos[2]) {
                cameraOffset[0] -= offsetSpeed * multiplier;
                cameraOffset[2] -= offsetSpeed * multiplier;
            } else if (cameraOffset[0] >= pos[0] && cameraOffset[2] >= pos[2]) {
                cameraOffset[0] += offsetSpeed * multiplier;
                cameraOffset[2] -= offsetSpeed * multiplier;
            }
            double distFinish = cameraOffset[0] * cameraOffset[0] + cameraOffset[2] * cameraOffset[2];
            distHold = sqrt(distHold);
            distFinish = sqrt(distFinish);
            distFinish /= distHold;
            cameraOffset[0] /= distFinish;
            cameraOffset[2] /= distFinish;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_KP_MINUS]) {
            cameraOffset[0] *= 1.01f;  // Zoom out
            cameraOffset[1] *= 1.01f;
            cameraOffset[2] *= 1.01f;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_KP_PLUS]) {
            cameraOffset[0] *= 0.99f;  // Zoom in
            cameraOffset[1] *= 0.99f;
            cameraOffset[2] *= 0.99f;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_F]) {
            angles[0] -= rotateSpeed;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_R]) {
            angles[0] += rotateSpeed;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_G]) {
            angles[1] -= rotateSpeed;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_T]) {
            angles[1] += rotateSpeed;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_H]) {
            angles[2] -= rotateSpeed;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_Y]) {
            angles[2] += rotateSpeed;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_S] || controllerLeftStateX < -JOYSTICK_DEAD_ZONE) {
            angles[1] = -1.0f * angle - 90.0f;
            double xSpeed = sin((angles[1]) * (PI / 180));
            double ySpeed = cos((angles[1]) * (PI / 180));

            if (controllerLeftStateX < -JOYSTICK_DEAD_ZONE) {
                multiplier = (static_cast<float>(controllerLeftStateX * -1)) / 32767;
            }

            pos[0] += (xSpeed / 300) * multiplier;
            pos[2] += (ySpeed / 300) * multiplier;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_W] || controllerLeftStateX > JOYSTICK_DEAD_ZONE) {
            angles[1] = -1.0f * angle + 90.0f;
            double xSpeed = sin(angles[1] * (PI / 180));
            double ySpeed = cos(angles[1] * (PI / 180));
            if (controllerLeftStateX > JOYSTICK_DEAD_ZONE) {
                multiplier = (static_cast<float>(controllerLeftStateX)) / 32767;
            }
            pos[0] += (xSpeed / 300) * multiplier;
            pos[2] += (ySpeed / 300) * multiplier;
        }
        if ((currentGame->getKeystate()[SDL_SCANCODE_SPACE] || buttonCheckA) && pos[1] == 0) {
            fallspeed = -0.003f;
            // pos[1] += 0.05f;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_E]) {
            fallspeed = 0;
            pos[1] += speed;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_Q]) {
            pos[1] -= speed;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_A] || controllerLeftStateY < -JOYSTICK_DEAD_ZONE) {
            angles[1] = -1.0f * angle + 180.0f;
            double xSpeed = sin((angles[1]) * (PI / 180));
            double ySpeed = cos((angles[1]) * (PI / 180));

            if (controllerLeftStateY < -JOYSTICK_DEAD_ZONE) {
                multiplier = (static_cast<float>(controllerLeftStateY * -1)) / 32767;
            }

            pos[0] += (xSpeed / 300) * multiplier;
            pos[2] += (ySpeed / 300) * multiplier;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_D] || controllerLeftStateY > JOYSTICK_DEAD_ZONE) {
            angles[1] = -1.0f * angle;
            double xSpeed = sin((angles[1]) * (PI / 180));
            double ySpeed = cos((angles[1]) * (PI / 180));

            if (controllerLeftStateY > JOYSTICK_DEAD_ZONE) {
                multiplier = static_cast<float>(controllerLeftStateY) / 32767;
            }

            pos[0] += (xSpeed / 300) * multiplier;
            pos[2] += (ySpeed / 300) * multiplier;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_Z]) {
            currentScale += scaleSpeed;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_X]) {
            currentScale -= scaleSpeed;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_C]) {
            currentLuminance += 0.01f;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_V]) {
            currentLuminance -= 0.01f;
        }
        if (currentGame->getKeystate()[SDL_SCANCODE_P]) {
            currentGame->changeWindowMode(SDL_WINDOW_FULLSCREEN_DESKTOP);
        } else if (currentGame->getKeystate()[SDL_SCANCODE_O]) {
            currentGame->changeWindowMode(SDL_WINDOW_FULLSCREEN);
        } else if (currentGame->getKeystate()[SDL_SCANCODE_I]) {
            currentGame->changeWindowMode(0);
        }
        /// @todo Input is VERY primitive right now... The sleep will make this action feel sluggish,
        /// but it prevents key spamming
        if (currentGame->getKeystate()[SDL_SCANCODE_U]) {
            if (SDL_GetRelativeMouseMode() == SDL_FALSE) {
                SDL_SetRelativeMouseMode(SDL_TRUE);
                trackMouse = true;
                sleep(1);
            } else {
                SDL_SetRelativeMouseMode(SDL_FALSE);
                trackMouse = false;
                sleep(1);
            }
        }
        fallspeed = basicPhysics(&pos[1], fallspeed);
        // cout << "CO - X: " << cameraOffset[0] << ", Y: " << cameraOffset[1]
        //    << ", Z: " << cameraOffset[2] << "\n";
        // cout << "dx: " << mouseX << ", dy: " << mouseY << "\n";
        currentGameInfo->currentGame->lockScene();
        currentGameInfo->gameCamera->setOffset(cameraOffset);
        currentGame->setLuminance(currentLuminance);
        character->setRotation(angles);
        character->setPosition(pos);
        currentGameInfo->currentGame->unlockScene();
    }
    SDL_GameControllerClose(gameController1);
    return;
} //NOLINT - refactor required

/*
 (vector<double>) cameraDistance takes a 3D vector containing the offset of the
 camera from the object and calculates the distance between the two with
 respect to the y-z plane, and x-y plane. Returns a 2D vector where the first
 element is the y-z distance, and the second element is the x-y distance.
*/
vector<double> cameraDistance(vec3 offset) {
    vector<double> distance(2);
    distance[0] = offset[1] * offset[1] + offset[2] * offset[2];
    distance[1] = offset[1] * offset[1] + offset[0] * offset[0];
    return distance;
}

double convertNegToDeg(double degree) {
    return degree >= 0.0f ? degree : degree + 360.0f;
}

// Calculates the angle between two points in degrees
double angleOfPoint(vec3 p1, vec3 p2) {
    auto diffPoint = p2 - p1;
    double angle = atan2(diffPoint[2], diffPoint[0]) * (180.0f / PI);
    return convertNegToDeg(angle);
}
