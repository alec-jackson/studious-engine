/**
 * @file gameInstance.cpp
 * @author Christian Galvez
 * @brief Implementation of gameInstance
 * @version 0.1
 * @date 2023-07-28
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <SDL_gamecontroller.h>
#include <SDL_keyboard.h>
#include <SDL_scancode.h>
#include <condition_variable> //NOLINT
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <queue>
#include <GameInstance.hpp>
#include <SceneObject.hpp>

// GameInput maps for input devices
map<SDL_Scancode, GameInput> keyboardInputMap = {
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
map<SDL_GameControllerButton, GameInput> controllerInputMap = {
    { SDL_CONTROLLER_BUTTON_DPAD_UP, GameInput::NORTH },
    { SDL_CONTROLLER_BUTTON_DPAD_DOWN, GameInput::SOUTH },
    { SDL_CONTROLLER_BUTTON_DPAD_RIGHT, GameInput::EAST },
    { SDL_CONTROLLER_BUTTON_DPAD_LEFT, GameInput::WEST },
    { SDL_CONTROLLER_BUTTON_A, GameInput::A },
    { SDL_CONTROLLER_BUTTON_B, GameInput::B },
    { SDL_CONTROLLER_BUTTON_X, GameInput::X },
    { SDL_CONTROLLER_BUTTON_BACK, GameInput::QUIT }
};

map<Uint8, GameInput> hatInputMap = {
    { SDL_HAT_UP, GameInput::NORTH },
    { SDL_HAT_DOWN, GameInput::SOUTH },
    { SDL_HAT_LEFT, GameInput::WEST },
    { SDL_HAT_RIGHT, GameInput::EAST }
};

/*
 (void) startGameInstance uses the passed struct (gameInstanceArgs) args to
 configure the new GameInstance. The args struct is defined as the following:
 int windowWidth - Defines the width of the application window
 int windowHeight - Defines the height of the application window
 vector<string> soundList - Contains paths to sound effects to load
 vector<string> vertexShaders - Contains paths to vertex shaders
 vector<string> fragmentShaders - Contains paths to vertex shaders
 mutex &lock - Reference to lock used for locking all shared resources

 (void) startGameInstance does not return any value.
 */
GameInstance::GameInstance(GfxController *gfxController, AnimationController *animationController,
        int width, int height) : gfxController_ { gfxController }, animationController_ { animationController },
        width_ { width }, height_ { height },
        shutdown_(0) {
    luminance = 1.0f;  // Set default values
    directionalLight = vec3(-100, 100, 100);
    controllersConnected = 0;
    init();
}

void GameInstance::init() {
    initWindow();
    initAudio();
    initController();
    initApplication();
    keystate = SDL_GetKeyboardState(NULL);
}

/*
 (int) getWidth returns the current width of the single window for the current
 GameInstance.
 */
int GameInstance::getWidth() {
    return width_;
}

/*
 (int) getHeight returns the current height of the single window for the current
 GameInstance.
 */
int GameInstance::getHeight() {
    return height_;
}

vec3 GameInstance::getResolution() {
    return vec3(static_cast<float>(width_), static_cast<float>(height_), 0.0f);
}

/*
 (vec3) getDirectionalLight returns the current location of the directionalLight
 in the scene.
 */
vec3 GameInstance::getDirectionalLight() {
    return directionalLight;
}

bool GameInstance::getControllerInput(SDL_GameControllerButton button) const {
    if (!controllersConnected) return false;
    // Checks if a button was pressed against all connected controllers
    if (gameControllers[0] == nullptr) {
        return false;
    }
    return SDL_GameControllerGetButton(gameControllers[0], button);
}

bool GameInstance::getKeyboardInput(SDL_Scancode scancode) const {
    return keystate[scancode];
}

bool GameInstance::pollInput(GameInput input) {
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
controllerReadout* GameInstance::getControllers(int controllerIndex) {
    controllerInfo[controllerIndex].leftAxis =
        SDL_GameControllerGetAxis(gameControllers[controllerIndex],
        SDL_CONTROLLER_AXIS_LEFTY);
    return &controllerInfo[controllerIndex];
}

/*
 (int) getControllersConnected returns the number of controllers connected and
 detected by the current SDL instance.
*/
int GameInstance::getControllersConnected() {
    return controllersConnected;
}

/**
 * @brief Plays a loaded sound effect.
 * @param sfxName The name of the previously loaded sfx to play.
 * @param loop If set to true, the sound effect will repeat indefinitely.
 * @param volume Volume level to play sound effect at. Valid values are between
 * 0-128.
 * @return 0 on success, -1 on failure.
 */
int GameInstance::playSound(string sfxName, bool loop, int volume) {
    std::unique_lock<std::mutex> scopeLock(soundLock_);
    /* Make sure the sound has been previously loaded */
    auto lsit = loadedSounds_.find(sfxName);
    if (lsit == loadedSounds_.end()) {
        fprintf(stderr, "GameInstance::playSound: Cannot play %s! Sound has not been loaded.\n",
            sfxName.c_str());
        return -1;
    }
    if (audioInitialized_) {
        // Adjust the volume of the sound
        // Volume ranges from (0-128)
        // so (volume / 128) = % volume
        Mix_VolumeChunk(lsit->second, volume);
        activeChannels_[sfxName] = Mix_PlayChannel(-1, lsit->second, loop);
    } else {
        fprintf(stderr, "GameInstance::playSound: Sound uninitialized, not playing any sounds\n");
        return -1;
    }
    return 0;
}

/**
 * @brief Change the volume of a loaded sound effect.
 * @param sfxName The name of the sound effect to modify.
 * @param volume The volume to set for the sound effect. Range of 0-128.
 * @return 0 on success, -1 on failure.
 */
int GameInstance::changeVolume(string sfxName, int volume) {
    auto lsit = loadedSounds_.find(sfxName);
    if (lsit == loadedSounds_.end()) {
        fprintf(stderr, "GameInstance::changeVolume: Cannot change volume of %s, sound has not been loaded\n",
            sfxName.c_str());
        return -1;
    }
    if (audioInitialized_) {
        Mix_VolumeChunk(lsit->second, volume);
    } else {
        fprintf(stderr, "GameInstance::playSound: Sound uninitialized, cannot change volume!\n");
        return -1;
    }
    return 0;
}

/**
 * @brief Stops the sound effect's playback if it is active.
 * @param sfxName The name of the sfx to stop.
 * @return 0 on success and the sound is paused, -1 if a failure occurred.
 * A failure will occur if the sound is not currently active.
 */
int GameInstance::stopSound(string sfxName) {
    /* Check if the sfx is active */
    auto acit = activeChannels_.find(sfxName);
    if (acit != activeChannels_.end()) {
        Mix_HaltChannel(acit->second);
    } else {
        fprintf(stderr, "GameInstance::stopSound: %s is not playing\n",
            sfxName.c_str());
        return -1;
    }
    return 0;
}

/*
 (void) changeWindowMode currently only changes the current gameInstance window
 into fullscreen mode.

 (void) changeWindowMode does not return any value.
*/
void GameInstance::changeWindowMode(int mode) {
    // SDL_DisplayMode DM;
    // if(mode > 1){
    //     SDL_GetCurrentDisplayMode(0, &DM);
    // } else {
    //     DM.w = width;
    //     DM.h = height;
    // }
    // SDL_SetWindowSize(window, DM.w, DM.h);
    SDL_SetWindowFullscreen(window, mode);
    // SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

GameInstance::~GameInstance() {
    printf("GameInstance::~GameInstance\n");
    /* We should empty the protected gfx requests queue */
    runGfxRequests();
    for (int i = 0; i < controllersConnected; i++) {
        SDL_GameControllerClose(gameControllers[i]);
    }
    for (auto &s : loadedSounds_) {
        Mix_FreeChunk(s.second);
    }
    Mix_CloseAudio();
    loadedSounds_.clear();
    activeChannels_.clear();
    shutdown();
    return;
}

void GameInstance::shutdown() {
    if (isShutDown()) return;
    printf("GameInstance::shutdown %p\n", window);
    // If we haven't already called shutdown
    if (window != nullptr) {
        SDL_GL_DeleteContext(mainContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        window = nullptr;
    }
    // Complete all protectedGfxRequests
    runGfxRequests();
    shutdown_ = 1;
    // Notify condition variables of shutdown
    inputCv_.notify_all();
    progressCv_.notify_all();
}

bool GameInstance::protectedGfxRequest(std::function<void(void)> req) {
    // Do nothing when we shut down
    if (isShutDown()) return false;
    printf("GameInstance::protectedGfxRequest: Enter\n");
    // Obtain the scene lock to add the request
    std::unique_lock<std::mutex> scopeLock(requestLock_);
    std::condition_variable cv;
    std::atomic<int> done = 0;
    auto reqCb = [req, &done, &cv]() {
        // Call the request
        req();
        // Set the done flag and signal completion
        done = 1;
        cv.notify_all();
    };
    // Add the request to the request queue
    protectedGfxReqs_.push(reqCb);
    // Wait for the reqLock to become available
    printf("GameInstance::protectedGfxRequest: Added request, waiting for completion...\n");
    cv.wait(scopeLock, [&done, this]() { return done == 1 || isShutDown(); });
    printf("GameInstance::protectedGfxRequest: Exit\n");
    return done == 1;  // Wakeup thread making call
}

/* The scene lock should be captured when entering this function */
void GameInstance::runGfxRequests() {
    std::unique_lock<std::mutex> scopeLock(requestLock_);
    while (!protectedGfxReqs_.empty()) {
        auto cb = protectedGfxReqs_.front();
        cb();
        protectedGfxReqs_.pop();
    }
}

/*
 (int) updateObjects updates properties for all of the active GameObjects in the
 current scene. Things that are updated include:
 * Directional light position for lighting
 * Luminance values for lighting intensity
 * VP Matrix per object
 After updating values for objects, the object is rendered to the SDL window
 surface.

 On success, 0 is returned and the GameInstance scene is rendered. On failure,
 -1 is returned and nothing is rendered.
*/
int GameInstance::updateObjects() {
    /* Run requests in the protectedGfxRequest queue */
    runGfxRequests();
    std::unique_lock<std::mutex> lock(sceneLock_);
    // Run the protected queue functions here - the scene lock protects it
    if (cameras_.empty()) {
        cerr << "Error: No active Cameras in the scene!\n";
        return -1;
    }
    gfxController_->update();
    // Update cameras
    for (auto camera : cameras_) {
        camera->setResolution(this->getResolution());
        camera->update();
    }
    return 0;
}

/*
 (int) updateWindow updates the current SDL window, showing updates to the
 OpenGL instance.

 (int) updateWindow returns 0 upon success. Error codes may be added to this
 method sometime in the future, thus why it is not void.
*/
int GameInstance::updateWindow() {
    SDL_GL_SwapWindow(window);
    // Retrieve the current window resolution
    SDL_GetWindowSize(window, &width_, &height_);
    return 0;
}

void GameInstance::updateInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_KEYDOWN) {
            // Lock access to the input queue
            std::unique_lock<std::mutex> scopeLock(inputLock_);
            // printf("Keyboard pressed %d\n", event.key.keysym.scancode);
            // Let's just use the queue as a mailbox for now
            auto input = scancodeToInput(event.key.keysym.scancode);
            if (inputQueue_.empty() && input != GameInput::NONE) {
                inputQueue_.push(input);
            }
            // Signal data is available
            inputCv_.notify_all();
        } else if (event.type == SDL_JOYBUTTONDOWN) {
            // Lock access to the input queue
            std::unique_lock<std::mutex> scopeLock(inputLock_);
            // printf("Button pressed %d\n", event.jbutton.button);
            // Let's just use the queue as a mailbox for now
            auto input = buttonToInput(static_cast<SDL_GameControllerButton>(event.jbutton.button));
            if (inputQueue_.empty() && input != GameInput::NONE) {
                inputQueue_.push(input);
            }
            // Signal data is available
            inputCv_.notify_all();
        } else if (event.type == SDL_JOYHATMOTION) {
            // Lock access to the input queue
            std::unique_lock<std::mutex> scopeLock(inputLock_);
            // printf("Hat pressed %d\n", event.jhat.value);
            // Let's just use the queue as a mailbox for now
            auto input = hatToInput(static_cast<Uint8>(event.jhat.value));
            if (inputQueue_.empty() && input != GameInput::NONE) {
                inputQueue_.push(input);
            }
            // Signal data is available
            inputCv_.notify_all();
        } else if (event.type == SDL_QUIT) {
            shutdown();
        } else if (event.type == SDL_JOYDEVICEADDED || event.type == SDL_JOYDEVICEREMOVED) {
            // Connect to new controllers on the fly...
            resetController();
            initController();
        }
    }
}

GameInput GameInstance::hatToInput(Uint8 hatValue) {
    auto input = GameInput::NONE;
    // Check the input map for a game input
    auto cimit = hatInputMap.find(hatValue);
    if (cimit != hatInputMap.end()) {
        input = cimit->second;
    }
    return input;
}

GameInput GameInstance::getInput() {
    auto res = GameInput::NONE;
    queue<GameInput> blankQueue;
    std::unique_lock<std::mutex> scopeLock(inputLock_);
    inputQueue_.swap(blankQueue);
    // Check if any items are in the input queue
    inputCv_.wait(scopeLock, [this]() { return !inputQueue_.empty() || shutdown_; });
    // Pop the item off of the queue
    if (!inputQueue_.empty()) {
        res = inputQueue_.front();
        inputQueue_.pop();
    }
    return res;
}

bool GameInstance::waitForInput(GameInput input) {
    auto curInput = GameInput::NONE;
    while (!shutdown_) {
        curInput = getInput();
        if (input == curInput) break;
    }
    return curInput == input;
}

GameObject *GameInstance::createGameObject(Polygon *characterModel, vec3 position, vec3 rotation, float scale,
    string objectName) {
    std::unique_lock<std::mutex> lock(sceneLock_);
    printf("GameInstance::createGameObject: Creating GameObject %zu\n", sceneObjects_.size());
    auto gameObjProg = gfxController_->getProgramId(GAMEOBJECT_PROG_NAME);
    if (!gameObjProg.isOk()) {
        fprintf(stderr,
            "GameInstance::createGameObject: Failed to create GameObject! '%s' program does not exist!\n",
            GAMEOBJECT_PROG_NAME);
        return nullptr;
    }
    auto gameObject = std::make_shared<GameObject>(characterModel, position, rotation,
        scale, gameObjProg.get(), objectName, ObjectType::GAME_OBJECT, gfxController_);
    gameObject.get()->setDirectionalLight(directionalLight);
    gameObject.get()->setLuminance(luminance);
    gameObject.get()->setRenderPriority(RENDER_PRIOR_LOW);
    sceneObjects_.push_back(gameObject);
    return gameObject.get();
}

CameraObject *GameInstance::createCamera(SceneObject *target, vec3 offset, float cameraAngle, float aspectRatio,
              float nearClipping, float farClipping) {
    std::unique_lock<std::mutex> lock(sceneLock_);
    printf("GameInstance::createCamera: Creating CameraObject %zu\n", sceneObjects_.size());
    auto cameraName = "Camera" + std::to_string(sceneObjects_.size());
    auto gameCamera = std::make_shared<CameraObject>(target, offset, cameraAngle,
        aspectRatio, nearClipping, farClipping, ObjectType::CAMERA_OBJECT, cameraName, gfxController_);
    cameras_.push_back(gameCamera);
    return gameCamera.get();
}

TextObject *GameInstance::createText(string message, vec3 position, float scale, string fontPath,
    float charSpacing, int charPoint, string objectName) {
    std::unique_lock<std::mutex> lock(sceneLock_);
    printf("GameInstance::createText: Creating TextObject %zu\n", sceneObjects_.size());
    auto textProg = gfxController_->getProgramId(TEXTOBJECT_PROG_NAME);
    if (!textProg.isOk()) {
        fprintf(stderr,
            "GameInstance::createText: Failed to create TextObject! '%s' program does not exist!\n",
            TEXTOBJECT_PROG_NAME);
        return nullptr;
    }
    auto text = std::make_shared<TextObject>(message, position, scale, fontPath,
        charSpacing, charPoint, textProg.get(), objectName, ObjectType::TEXT_OBJECT, gfxController_);
    text.get()->setRenderPriority(RENDER_PRIOR_HIGH);
    sceneObjects_.push_back(text);
    return text.get();
}

SpriteObject *GameInstance::createSprite(string spritePath, vec3 position, float scale,
    ObjectAnchor anchor, string objectName) {
    std::unique_lock<std::mutex> lock(sceneLock_);
    auto spriteProg = gfxController_->getProgramId(SPRITEOBJECT_PROG_NAME);
    if (!spriteProg.isOk()) {
        fprintf(stderr,
            "GameInstance::createSprite: Unable to create sprite! '%s' program does not exist!\n",
            SPRITEOBJECT_PROG_NAME);
        return nullptr;
    }
    auto sprite = std::make_shared<SpriteObject>(spritePath, position, scale, spriteProg.get(), objectName,
        ObjectType::SPRITE_OBJECT, anchor, gfxController_);
    sprite.get()->setRenderPriority(RENDER_PRIOR_LOW);
    sceneObjects_.push_back(sprite);
    return sprite.get();
}

UiObject *GameInstance::createUi(string spritePath, vec3 position, float scale, float wScale, float hScale,
    ObjectAnchor anchor, string objectName) {
    std::unique_lock<std::mutex> lock(sceneLock_);
    auto uiProg = gfxController_->getProgramId(UIOBJECT_PROG_NAME);
    if (!uiProg.isOk()) {
        fprintf(stderr,
            "GameInstance::createUi: Failed to create UI object! '%s' program does not exist!\n",
            UIOBJECT_PROG_NAME);
        return nullptr;
    }
    auto ui = std::make_shared<UiObject>(spritePath, position, scale, wScale, hScale, uiProg.get(), objectName,
        ObjectType::UI_OBJECT, anchor, gfxController_);
    ui.get()->setRenderPriority(RENDER_PRIOR_MEDIUM);
    sceneObjects_.push_back(ui);
    return ui.get();
}

TileObject *GameInstance::createTileMap(map<string, string> textures, vector<TileData> mapData,
    vec3 position, float scale, ObjectAnchor anchor, string objectName) {
    std::unique_lock<std::mutex> lock(sceneLock_);
    printf("GameInstance::createTileMap: Creating TextObject %zu\n", sceneObjects_.size());
    auto tileProg = gfxController_->getProgramId(TILEOBJECT_PROG_NAME);
    if (!tileProg.isOk()) {
        fprintf(stderr,
            "GameInstance::createTileMap: Failed to create tile map! '%s' program does not exist!\n",
            TILEOBJECT_PROG_NAME);
        return nullptr;
    }
    auto tile = std::make_shared<TileObject>(textures, mapData, position, vec3(0.0f), scale, ObjectType::TILE_OBJECT,
    tileProg.get(), objectName, anchor, gfxController_);
    tile.get()->setRenderPriority(RENDER_PRIOR_LOWEST);
    sceneObjects_.push_back(tile);
    return tile.get();
}

SceneObject *GameInstance::getSceneObject(string objectName) {
    std::unique_lock<std::mutex> lock(sceneLock_);
    SceneObject *result = nullptr;
    for (auto it = sceneObjects_.begin(); it != sceneObjects_.end(); ++it) {
        if ((*it).get()->getObjectName().compare(objectName) == 0) result = (*it).get();
    }
    return result;
}

int GameInstance::update() {
    // Update any controllers here that should be paced with the frame rate
    int error = 0;
    error = updateObjects();
    error |= updateWindow();
    updateInput();
    animationController_->update();
    return error;
}

int GameInstance::removeSceneObject(string objectName) {
    std::unique_lock<std::mutex> lock(sceneLock_);
    animationController_->removeSceneObject(objectName);
    // Search for the object by name
    auto objectIt = std::find_if(sceneObjects_.begin(), sceneObjects_.end(),
        [&objectName](std::shared_ptr<SceneObject> obj) {
            return obj->getObjectName().compare(objectName) == 0;
        });

    if (objectIt == sceneObjects_.end()) {
        fprintf(stderr, "GameInstance::removeSceneObject: Not found (%s)\n",
            objectName.c_str());
        return -1;
    }
    for (auto camera : cameras_) {
        camera->removeSceneObject(objectName);
    }
    sceneObjects_.erase(objectIt);
    return 0;
}

/*
 (int) getCollision takes a (GameObject *) object1 and compares it to
 (GameObject *) object2 and checks whether the two GameObjects are either
 colliding or about to collide. getCollision checks whether the two GameObjects
 are about to collide using the (vec3) moving parameter. The general formula for
 this function is thefollowing:
 1. A collision is only currently occuring if object1's center point is within
    range of object2's center point. Range is dictacted by the sum of the two
    offset points for the corresponding axis.
 2. A collision is about to occur if object1's center point + (vec3) moving is
    within range of object2's center point.

 On success, (int) getCollision will return 1 if the two GameObjects are
 currently colliding, 2 if the two objects are about to collide, or 0 if there
 is no collision. Otherwise, -1 is returned.
*/
/// @todo Update documentation here and convert pointers to references
int GameInstance::getCollision(GameObject *object1, GameObject *object2,
    vec3 moving) {
    return object1->getCollider()->getCollision(object2->getCollider(), moving);
}

/**
 * 2D version of the same function defined above.
 */
int GameInstance::getCollision2D(GameObject2D *object1, GameObject2D *object2, vec3 moving) {
    return object1->getCollider()->getCollision(object2->getCollider(), moving);
}

/*
 (void) setLuminance sets the current GameInstance's luminance value to the
 provided (float) luminanceValue.
*/
void GameInstance::setLuminance(float luminanceValue) {
    luminance = luminanceValue;
}

/*
 (void) initWindow creates a new SDL window instance using the given
 (int) height and (int) width to determine the size of the window in pixels.

 (void) initWindow does not return any values.
*/
void GameInstance::initWindow() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
    window = SDL_CreateWindow("Studious Engine Example", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, width_, height_,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
#ifndef GFX_EMBEDDED
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif
#ifdef __APPLE__  // Temporarily restrict SDL AA to MACOS
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, AASAMPLES);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, AASAMPLES);
#endif
    mainContext = SDL_GL_CreateContext(window);
    if (!mainContext) {
        fprintf(stderr, "GameInstance::initWindow: Failed to init Context!\n");
        exit(EXIT_FAILURE);
    }
    renderer = SDL_GetRenderer(window);

    if (window == NULL) {
        cerr << "Error: Failed to create SDL window!\n";
        return;
    }
}

void GameInstance::configureVsync(bool enable) {
    SDL_GL_SetSwapInterval(enable);  // 0 - Disable VSYNC / 1 - Enable VSYNC
}

/**
 * @brief Attempts to initialize the audio with SDL2-mixer.
 * @note On success, the audioInitialized_ flag is set to true.
 */
void GameInstance::initAudio() {
    audioID = Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 512);
    if (audioID < 0) {
        cerr << "Error: Unable to open audio: " << SDL_GetError() <<
            "\n";
        return;
    }
    audioID = Mix_AllocateChannels(4);
    if (audioID < 0) {
        cerr << "Error: Unable to allocate mixing channels: "
            << SDL_GetError() << "\n";
        return;
    }
    audioInitialized_ = true;
}

/**
 * @brief Loads a sound into memory. The loaded sound becomes available in the
 * loadedSounds_ map.
 * @param sfxName Friendly name of the sound effect.
 * @param sfxPath Path to the sound effect file to load.
 * @return 0 on success, -1 on failure.
 */
int GameInstance::loadSound(string sfxName, string sfxPath) {
    auto sfxData = Mix_LoadWAV(sfxPath.c_str());
    if (sfxData == nullptr) {
        fprintf(stderr, "GameInstance::loadSound: Failed to load sfx %s at %s\n",
            sfxName.c_str(), sfxPath.c_str());
        return -1;
    }
    loadedSounds_[sfxName] = sfxData;
    return 0;
}

/*
 (void) initController attemps to initialize any connected joysticks with the
 current SDL instance.

 (void) initController does not return any values.
*/
void GameInstance::initController() {
    std::unique_lock<std::mutex> scopeLock(controllerLock_);
    int joyFlag = SDL_NumJoysticks();
    cout << "Number of joysticks connected: " << joyFlag << "\n";
    if (joyFlag < 1) {
        cout << "Warning: No Joysticks Detected\n";
    } else {
        for (int i = 0; i < joyFlag; i++) {
            if (SDL_IsGameController(i)) {
                gameControllers[controllersConnected] = SDL_GameControllerOpen(i);
                if (gameControllers[controllersConnected] == NULL) {
                    cerr << "Error: Unable to open game controller - "
                        << SDL_GetError() << "\n";
                } else {
                    controllersConnected++;
                    return;
                }
            }
        }
        cout << "No available Xinput joysticks detected!\n";
    }
    return;
}

void GameInstance::resetController() {
    std::unique_lock<std::mutex> scopeLock(controllerLock_);
    for (int i = 0; i < controllersConnected; ++i) {
        SDL_GameControllerClose(gameControllers[i]);
        gameControllers[i] = nullptr;
    }
    controllersConnected = 0;
}

void GameInstance::initApplication() {
    gfxController_->init();
}

int GameInstance::lockScene() {
    sceneLock_.lock();
    return 0;
}

int GameInstance::unlockScene() {
    sceneLock_.unlock();
    return 0;
}

bool GameInstance::waitForProgress(std::function<bool(void)> pred) {
    std::unique_lock<std::mutex> scopeLock(progressLock_);
    progressCv_.wait(scopeLock, [pred, this]() { return pred() || isShutDown(); });
    return !isShutDown();
}

GameInput GameInstance::scancodeToInput(SDL_Scancode scancode) {
    auto input = GameInput::NONE;
    // Check the input map for a game input
    auto cimit = keyboardInputMap.find(scancode);
    if (cimit != keyboardInputMap.end()) {
        input = cimit->second;
    }
    return input;
}

GameInput GameInstance::buttonToInput(SDL_GameControllerButton button) {
    auto input = GameInput::NONE;
    // Check the input map for a game input
    auto cimit = controllerInputMap.find(button);
    if (cimit != controllerInputMap.end()) {
        input = cimit->second;
    }
    return input;
}

const Uint8 *GameInstance::getKeystateRaw() {
    return keystate;
}
