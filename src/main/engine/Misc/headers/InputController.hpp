/**
 * @file inputController.hpp
 * @author Christian Galvez
 * @author Alec Jackson
 * @brief Helper class to monitor input
 * @version 0.1
 * @date 2025
 *
 * @copyright Copyright (c) 2025
 *
 */

#pragma once

#include <mutex> //NOLINT

enum class GameInput {
    NONE,
    QUIT,
    NORTH,
    SOUTH,
    EAST,
    WEST,
    A,
    B,
    X,
    Y,
    R,
    L
};

/*
 controllerReadout is used for getting input from a controller. This struct
 will be used in conjunction with SDL_GameControllerGetAxis to get input from
 the left controller stick.
*/
typedef struct controllerReadout {
    Sint16 leftAxis;
} controllerReadout;

class InputController {
	private:
	const Uint8 *keystate;
	int controllersConnected;
	SDL_GameController *gameControllers[2];
	controllerReadout controllerInfo[2];

	std::mutex controllerLock_;

	public:
	InputController();
	~InputController();
	/**
	 * @brief Can be used to check raw SDL scancode values. This has the same behavior as the previous
	 * getKeystate function did.
	 * @return Uint8 array that can be indexed using SDL_Scancode enumerated values.
	 */
	const Uint8 *getKeystateRaw();
	/**
	 * @brief Shorthand way to check if a keyboard button has been pressed.
	 * @param scancode - The SDL_Scancode enum value to check for input state.
	 * @return true if the button described in scancode is pressed down, false otherwise.
	 */
	bool getKeyboardInput(SDL_Scancode scancode) const;
	/**
	 * @brief Check if a SDL_GameControllerButton has been pressed.
	 * @param button - The button to poll the input state of. Described by SDL_GameControllerButton.
	 * @return true if the button is pressed down, false otherwise.
	 */
	bool getControllerInput(SDL_GameControllerButton button) const;
	/**
	 * @brief Polls for a specific input across keyboard and controller input devices.
	 * @param input - The input to check for.
	 * @return true if the GameInput is being pressed by a keyboard or controller. False otherwise.
	 */
	bool pollInput(GameInput input);

	/*
	 (controllerReadout *) getControllers takes an (int) controllerIndex and returns
	 the associated controllerReadout struct.
	*/
	controllerReadout *getControllers(int controllerIndex);

	/*
	 (int) getControllersConnected returns the number of controllers connected and
	 detected by the current SDL instance.
	*/
	int getControllersConnected();

    /**
     * @brief Converts a raw SDL scancode value to a GameInput value. @see keyboardInputMap in the GameInstance.cpp
     * source file to see the complete mapping.
     * @return GameInput mapped to the raw input, or GameInput::NONE if undefined input received.
     */
    GameInput scancodeToInput(SDL_Scancode scancode);
    /**
     * @brief Converts a raw SDL button input to the raw input's button map. @see controllerInputMap in the
     * GameInstance.cpp source file.
     * @return GameInput mapping to the raw button input. Returns GameInput::NONE if an input is received that is
     * not defined in the controllerInputMap.
     */
    GameInput buttonToInput(SDL_GameControllerButton button);
    /**
     * @brief Converts a raw SDL hat input to a GameInput.
     * @return GameInput mapping to the raw hat value. Returns GameInput::NONE is mapping not found.
     */
    GameInput hatToInput(Uint8 hatValue);
    /**
     * @brief Closes all active controllers and performs some cleanup.
     */
    void resetController();

	void initController();
};