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
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <GameInstance.hpp>

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
GameInstance::GameInstance(vector<string> vertShaders,
        vector<string> fragShaders, GfxController *gfxController, int width, int height)
        : gfxController_ { gfxController }, vertShaders_ { vertShaders },
        fragShaders_ { fragShaders }, width_ { width }, height_ { height }, shutdown_ ( 0 ) {
    luminance = 1.0f;  // Set default values
    directionalLight = vec3(-100, 100, 100);
    controllersConnected = 0;
}

// Helper function for startup
void GameInstance::startGame(const configData &config) {
    initWindow(config);
    initAudio();
    initController();
    initApplication(vertShaders_, fragShaders_);
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

/*
 (const Uint8 *) getKeystate returns the current keystate of the current
 GameInstance. The keystate is used for getting input from the user's keyboard.
 */
const Uint8 *GameInstance::getKeystate() {
    return keystate;
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
    printf("GameInstance::shutdown %p\n", window);
    // If we haven't already called shutdown
    if (window != nullptr) {
        SDL_GL_DeleteContext(mainContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        window = nullptr;
    }
    shutdown_ = 1;
    // Notify condition variables of shutdown
    inputCv_.notify_all();
}

/*
 (bool) isWindowOpen checks if the current SDL window is currently open still.

 (bool) isWindowOpen returns true when the SDL window is still open, otherwise
 false is returned.
*/
bool GameInstance::isWindowOpen() {
    return !keystate[SDL_SCANCODE_ESCAPE];
}

void GameInstance::protectedGfxRequest(std::function<void(void)> req) {
    printf("GameInstance::protectedGfxRequest: Enter\n");
    // Obtain the scene lock to add the request
    std::unique_lock<std::mutex> scopeLock(requestLock_);
    std::mutex reqLock;
    reqLock.lock();
    auto reqCb = [req, &reqLock]() {
        // Call the request
        req();
        // Then unlock the reqLock
        reqLock.unlock();
    };
    // Add the request to the request queue
    protectedGfxReqs_.push(reqCb);
    scopeLock.unlock();
    // Wait for the reqLock to become available
    printf("GameInstance::protectedGfxRequest: Added request, waiting on lock...\n");
    reqLock.lock();
    printf("GameInstance::protectedGfxRequest: Exit\n");
    return;  // Wakeup thread making call
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
    if (sceneObjects_.empty()) {
        cerr << "Error: No active GameObjects in the current scene!\n";
        return -1;
    }
    gfxController_->update();
    // Update cameras
    for (auto it = sceneObjects_.begin(); it != sceneObjects_.end(); ++it) {
        if ((*it).get()->type() == ObjectType::CAMERA_OBJECT) {
            CameraObject *cObj = static_cast<CameraObject *>((*it).get());
            // Send the current screen resolution to the camera
            cObj->setResolution(this->getResolution());
            cObj->update();
        }
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
    updateInput();
    return 0;
}

void GameInstance::updateInput() {
    SDL_Event event;
    auto res = SDL_PollEvent(&event);
    if (res && event.type == SDL_KEYDOWN) {
        // Lock access to the input queue
        std::unique_lock<std::mutex> scopeLock(inputLock_);
        // Let's just use the queue as a mailbox for now
        if (inputQueue_.empty()) {
            inputQueue_.push(event.key.keysym.scancode);
        }
        // Signal data is available
        inputCv_.notify_all();
    }
}

SDL_Scancode GameInstance::getInput() {
    auto res = SDL_SCANCODE_UNKNOWN;
    std::unique_lock<std::mutex> scopeLock(inputLock_);
    // Check if any items are in the input queue
    inputCv_.wait(scopeLock, [this]() { return !inputQueue_.empty() || shutdown_; });
    // Pop the item off of the queue
    if (!inputQueue_.empty()) {
        res = inputQueue_.front();
        inputQueue_.pop();
    }
    return res;
}

bool GameInstance::waitForKeyDown(SDL_Scancode input) {
    SDL_Scancode curInput = SDL_SCANCODE_UNKNOWN;
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
    auto gameObject = std::make_shared<GameObject>(characterModel, position, rotation,
        scale, objectName, ObjectType::GAME_OBJECT, gfxController_);
    gameObject.get()->setDirectionalLight(directionalLight);
    gameObject.get()->setLuminance(luminance);
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
    sceneObjects_.push_back(gameCamera);
    return gameCamera.get();
}

TextObject *GameInstance::createText(string message, vec3 position, float scale, string fontPath,
    float charSpacing, int charPoint, uint programId, string objectName) {
    std::unique_lock<std::mutex> lock(sceneLock_);
    printf("GameInstance::createText: Creating TextObject %zu\n", sceneObjects_.size());
    auto text = std::make_shared<TextObject>(message, position, scale, fontPath,
        charSpacing, charPoint, programId, objectName, ObjectType::TEXT_OBJECT, gfxController_);
    sceneObjects_.push_back(text);
    return text.get();
}

SpriteObject *GameInstance::createSprite(string spritePath, vec3 position, float scale, unsigned int programId,
    ObjectAnchor anchor, string objectName) {
    std::unique_lock<std::mutex> lock(sceneLock_);
    auto sprite = std::make_shared<SpriteObject>(spritePath, position, scale, programId, objectName,
        ObjectType::SPRITE_OBJECT, anchor, gfxController_);
    sceneObjects_.push_back(sprite);
    return sprite.get();
}

UiObject *GameInstance::createUi(string spritePath, vec3 position, float scale, float wScale, float hScale,
    unsigned int programId, ObjectAnchor anchor, string objectName) {
    std::unique_lock<std::mutex> lock(sceneLock_);
    auto ui = std::make_shared<UiObject>(spritePath, position, scale, wScale, hScale, programId, objectName,
        ObjectType::UI_OBJECT, anchor, gfxController_);
    sceneObjects_.push_back(ui);
    return ui.get();
}

SceneObject *GameInstance::getSceneObject(string objectName) {
    std::unique_lock<std::mutex> lock(sceneLock_);
    SceneObject *result = nullptr;
    for (auto it = sceneObjects_.begin(); it != sceneObjects_.end(); ++it) {
        if ((*it).get()->getObjectName().compare(objectName) == 0) result = (*it).get();
    }
    return result;
}

int GameInstance::removeSceneObject(string objectName) {
    std::unique_lock<std::mutex> lock(sceneLock_);
    // Search for the object by name
    auto objectIt = std::find_if(sceneObjects_.begin(), sceneObjects_.end(),
        [&objectName](std::shared_ptr<SceneObject> obj) {
            return obj->getObjectName().compare(objectName) == 0;
        });

    if (objectIt == sceneObjects_.end()) {
        fprintf(stderr, "GameInstance::removeSceneObject: Not found (%s)\n",
            objectName.c_str());
        return -1;
    } else {
        // This could be optimized a bit better - remove scene object from all cameras if found
        for (auto it = sceneObjects_.begin(); it != sceneObjects_.end(); ++it) {
            if ((*it).get()->type() == ObjectType::CAMERA_OBJECT) {
                // Attempt to remove the object from the current camera
                auto camera = static_cast<CameraObject *>((*it).get());
                camera->removeSceneObject((*objectIt).get()->getObjectName());
            }
        }
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
void GameInstance::initWindow(const configData &config) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
    window = SDL_CreateWindow("Studious Engine Example", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, width_, height_,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
#ifndef GFX_EMBEDDED
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif
#ifdef __APPLE__  // Temporarily restrict SDL AA to MACOS
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, AASAMPLES);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, AASAMPLES);
#endif
    mainContext = SDL_GL_CreateContext(window);
    if (!mainContext) exit(EXIT_FAILURE);

    SDL_GL_SetSwapInterval(config.enableVsync);  // 0 - Disable VSYNC / 1 - Enable VSYNC
    renderer = SDL_GetRenderer(window);

    if (window == NULL) {
        cerr << "Error: Failed to create SDL window!\n";
        return;
    }
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

/*
 (void) initApplication grabs the shader files provided in the
 (vector<string>) vertexPath and (vector<string>) fragmentPath vectors and
 compiles and loads them into the current GameInstance.

 (void) initApplication does not return any values.
*/
void GameInstance::initApplication(vector<string> vertexPath, vector<string> fragmentPath) {
    // Compile each of our shaders and assign them their own programId number
    gfxController_->init();
    for (uint i = 0; i < vertexPath.size(); i++) {
        gfxController_->loadShaders(vertexPath[i].c_str(), fragmentPath[i].c_str());
    }
}

/* [NOT IMPLEMENTED] [OLD WORK REMOVED DUE TO CHANGES]
 (void) basicCollision takes a (GameInstance *) gameInstance and performs a
 basic collision check on all of the active GameObjects in the scene. This
 method is still a WIP and does not really do anything at the moment.

 (void) basicCollision does not return any values.
*/
void GameInstance::basicCollision(GameInstance *gameInstance) {
}

int GameInstance::lockScene() {
    sceneLock_.lock();
    return 0;
}

int GameInstance::unlockScene() {
    sceneLock_.unlock();
    return 0;
}
