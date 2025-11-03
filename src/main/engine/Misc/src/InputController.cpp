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

#include <SDL_gamecontroller.h>
#include <SDL_keyboard.h>
#include <SDL_scancode.h>

#include <map>
#include <mutex>
#include <iostream>
#include <InputController.hpp>

// GameInput maps for input devices
std::map<SDL_Scancode, GameInput> keyboardInputMap = {
    { SDL_SCANCODE_W, GameInput::NORTH      },
    { SDL_SCANCODE_S, GameInput::SOUTH      },
    { SDL_SCANCODE_D, GameInput::EAST       },
    { SDL_SCANCODE_A, GameInput::WEST       },
    { SDL_SCANCODE_RETURN, GameInput::A     },
    { SDL_SCANCODE_BACKSPACE, GameInput::B  },
    { SDL_SCANCODE_BACKSPACE, GameInput::X  },
    { SDL_SCANCODE_ESCAPE, GameInput::QUIT  }
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
    { SDL_CONTROLLER_BUTTON_BACK, GameInput::QUIT }
};

std::map<Uint8, GameInput> hatInputMap = {
    { SDL_HAT_UP, GameInput::NORTH },
    { SDL_HAT_DOWN, GameInput::SOUTH },
    { SDL_HAT_LEFT, GameInput::WEST },
    { SDL_HAT_RIGHT, GameInput::EAST }
};

InputController::InputController() {
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
