/**
 * @file inputController.cpp
 * @author Alec Jackson
 * @author Christian Galvez
 * @brief Helper Class to monitor input
 * @version 0.1
 * @date 2025
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "ComplexCameraObject.hpp"
#include <InputController.hpp>
#include <SDL_gamecontroller.h>
#include <cstdint>
#include <map>
#include <mutex>  //NOLINT
#include <iostream>

// GameInput maps for input devices
std::map<SDL_Scancode, GameInput> keyboardInputMap = {
    { SDL_SCANCODE_W, GameInput::NORTH      },
    { SDL_SCANCODE_S, GameInput::SOUTH      },
    { SDL_SCANCODE_D, GameInput::EAST       },
    { SDL_SCANCODE_A, GameInput::WEST       },
    { SDL_SCANCODE_RETURN, GameInput::A     },
    { SDL_SCANCODE_BACKSPACE, GameInput::B  },
    { SDL_SCANCODE_E, GameInput::Y          },
    { SDL_SCANCODE_ESCAPE, GameInput::QUIT  },
    { SDL_SCANCODE_TAB, GameInput::START    }
};

// GameInput maps for input devices
std::map<SDL_GameControllerButton, GameInput> controllerInputMap = {
    { SDL_CONTROLLER_BUTTON_DPAD_UP, GameInput::NORTH },
    { SDL_CONTROLLER_BUTTON_DPAD_DOWN, GameInput::SOUTH },
    { SDL_CONTROLLER_BUTTON_DPAD_RIGHT, GameInput::EAST },
    { SDL_CONTROLLER_BUTTON_DPAD_LEFT, GameInput::WEST },
    { SDL_CONTROLLER_BUTTON_A, GameInput::A },
    { SDL_CONTROLLER_BUTTON_B, GameInput::B },
    { SDL_CONTROLLER_BUTTON_X, GameInput::X },
    { SDL_CONTROLLER_BUTTON_Y, GameInput::Y },
    { SDL_CONTROLLER_BUTTON_BACK, GameInput::QUIT },
    { SDL_CONTROLLER_BUTTON_START, GameInput::START }
};

std::map<Uint8, GameInput> hatInputMap = {
    { SDL_HAT_UP, GameInput::NORTH },
    { SDL_HAT_DOWN, GameInput::SOUTH },
    { SDL_HAT_LEFT, GameInput::WEST },
    { SDL_HAT_RIGHT, GameInput::EAST }
};

#define INVERT_MODIFIER(flag) if (flag) modifier *= -1.0f
#define TRACK_TRANSFORM TRACKING_SPEED * modifier * deltaTime

InputController::InputController(VEC(SHD(CameraObject)) &cameras, MUT &cameraLock) :
    cameras_ { cameras }, cameraLock_ { cameraLock } {
    std::cout << "Creating Controllers!\n";
    controllersConnected = 0;
    keystate = SDL_GetKeyboardState(NULL);
}

InputController::~InputController() {
    for (int i = 0; i < controllersConnected; i++) {
        SDL_GameControllerClose(gameControllers[i]);
    }
}

const Uint8 *InputController::getKeystateRaw() {
    return keystate;
}

bool InputController::getKeyboardInput(SDL_Scancode scancode) const {
    return keystate[scancode];
}

bool InputController::getControllerInput(SDL_GameControllerButton button) const {
    if (!controllersConnected) return false;
    // Checks if a button was pressed against all connected controllers
    if (gameControllers[0] == nullptr) {
        return false;
    }
    return SDL_GameControllerGetButton(gameControllers[0], button);
}

bool InputController::pollInput(GameInput input) {
    std::unique_lock<std::mutex> scopeLock(controllerLock_);
    auto pressed = false;
    // Check for the target input from either a controller or keyboard (will improve later)
    SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;
    SDL_GameControllerButton button = SDL_CONTROLLER_BUTTON_INVALID;
    // Reverse the maps and find the corresponding raw input
    for (auto kbEntry : keyboardInputMap) {
        // Check if the value is equal and use that keycode
        if (kbEntry.second == input) {
            scancode = kbEntry.first;
            break;
        }
    }
    for (auto cEntry : controllerInputMap) {
        // Check if the value is equal and use that keycode
        if (cEntry.second == input) {
            button = cEntry.first;
            break;
        }
    }
    // Now finally POLL for events on all devices.
    pressed |= getControllerInput(button);
    pressed |= getKeyboardInput(scancode);
    return pressed;
}

/*
 (controllerReadout *) getControllers takes an (int) controllerIndex and returns
 the associated controllerReadout struct.
*/
controllerReadout* InputController::getControllers(int controllerIndex) {
    controllerInfo[controllerIndex].leftAxis =
        SDL_GameControllerGetAxis(gameControllers[controllerIndex],
        SDL_CONTROLLER_AXIS_LEFTY);
    return &controllerInfo[controllerIndex];
}

/*
 (int) getControllersConnected returns the number of controllers connected and
 detected by the current SDL instance.
*/
int InputController::getControllersConnected() {
    return controllersConnected;
}

/*
 (void) initController attemps to initialize any connected joysticks with the
 current SDL instance.

 (void) initController does not return any values.
*/
void InputController::initController() {
    std::unique_lock<std::mutex> scopeLock(controllerLock_);
    int joyFlag = SDL_NumJoysticks();
    std::cout << "Number of joysticks connected: " << joyFlag << "\n";
    if (joyFlag < 1) {
        std::cout << "Warning: No Joysticks Detected\n";
    } else {
        for (int i = 0; i < joyFlag; i++) {
            if (SDL_IsGameController(i)) {
                gameControllers[controllersConnected] = SDL_GameControllerOpen(i);
                if (gameControllers[controllersConnected] == NULL) {
                    std::cerr << "Error: Unable to open game controller - "
                        << SDL_GetError() << "\n";
                } else {
                    controllersConnected++;
                    return;
                }
            }
        }
        std::cout << "No available Xinput joysticks detected!\n";
    }
    return;
}

void InputController::resetController() {
    std::unique_lock<std::mutex> scopeLock(controllerLock_);
    for (int i = 0; i < controllersConnected; ++i) {
        SDL_GameControllerClose(gameControllers[i]);
        gameControllers[i] = nullptr;
    }
    controllersConnected = 0;
}

GameInput InputController::scancodeToInput(SDL_Scancode scancode) {
    auto input = GameInput::NONE;
    // Check the input map for a game input
    auto cimit = keyboardInputMap.find(scancode);
    if (cimit != keyboardInputMap.end()) {
        input = cimit->second;
    }
    return input;
}

GameInput InputController::buttonToInput(SDL_GameControllerButton button) {
    auto input = GameInput::NONE;
    // Check the input map for a game input
    auto cimit = controllerInputMap.find(button);
    if (cimit != controllerInputMap.end()) {
        input = cimit->second;
    }
    return input;
}

GameInput InputController::hatToInput(Uint8 hatValue) {
    auto input = GameInput::NONE;
    // Check the input map for a game input
    auto cimit = hatInputMap.find(hatValue);
    if (cimit != hatInputMap.end()) {
        input = cimit->second;
    }
    return input;
}

void InputController::update() {
    updateCameraControls();
}

void InputController::updateCameraControls() {
    int mouseX, mouseY;
    float xModifier = 0.0f, yModifier = 0.0f;
    Sint16 controllerRightStateY = 0;
    Sint16 controllerRightStateX = 0;
    // Do nothing in relative mouse mode
    if (!SDL_GetRelativeMouseMode()) {
        ignoreFirstUpdate_ = true;
        return;
    }
    // Allow the mouse to capture - prevents jitters when attaching to camera
    if (ignoreFirstUpdate_) {
        SDL_GetRelativeMouseState(&mouseX, &mouseY);
        ignoreFirstUpdate_ = false;
        return;
    }
    SDL_GetRelativeMouseState(&mouseX, &mouseY);
    /**
     * Eventually, keyboard and mouse input will be separate from controller input,
     * but for now we are keeping legacy behavior where kb and mouse take precedence.
     */
    std::unique_lock<MUT> scopeLock(controllerLock_);
    if (controllersConnected > 0) {
        assert(gameControllers[0] != nullptr);
        controllerRightStateY = SDL_GameControllerGetAxis(gameControllers[0],
            SDL_CONTROLLER_AXIS_RIGHTY);
        controllerRightStateX = SDL_GameControllerGetAxis(gameControllers[0],
            SDL_CONTROLLER_AXIS_RIGHTX);
    }
    scopeLock.unlock();
    xModifier = fabs(mouseX) / 5.0f;
    yModifier = fabs(mouseY) / 5.0f;
    // Determine which X/Y inputs to send to cameras
    if (mouseX == 0 && mouseY == 0) {
        // Convert controller to mouse coordinates
        // Check for deadzone
        xModifier = abs(controllerRightStateX) > JOYSTICK_DEAD_ZONE ? fabs(controllerRightStateX) : 0.0f;
        yModifier = abs(controllerRightStateY) > JOYSTICK_DEAD_ZONE ? fabs(controllerRightStateY) : 0.0f;
        xModifier /= INT16_MAX;  // Normalize between 0 and 1
        yModifier /= INT16_MAX;
    }
    // Send process input to camera
    std::unique_lock<MUT> camLock(cameraLock_);
    for (auto camera : cameras_) {
        // Check if camera is Complex
        auto compCam = std::dynamic_pointer_cast<ComplexCameraObject>(camera);
        if (compCam) {
            compCam->sendInput(xModifier, yModifier);
        }
    }
}
