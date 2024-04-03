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
GameInstance::GameInstance(vector<string> soundList, vector<string> vertShaders,
        vector<string> fragShaders, GfxController *gfxController, int width, int height)
        : gfxController_ { gfxController }, soundList_ { soundList }, vertShaders_ { vertShaders },
        fragShaders_ { fragShaders }, width_ { width }, height_ { height } {
    luminance = 1.0f;  // Set default values
    directionalLight = vec3(-100, 100, 100);
    controllersConnected = 0;
}

void GameInstance::startGame() {
    initWindow(width_, height_);
    initAudio();
    // playSound(0, 1);
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

/*
 (void) playSound takes an (int) soundIndex number associated with a specific
 sound piece loaded into the game instance and plays that sound (int) loop
 number of times. If indefinite looping is desired, (int) loop can be set to -1.
 Infinitely looping sounds must be stopped manually.

 (void) playSound does not return any value.
*/
void GameInstance::playSound(int soundIndex, int loop) {
    Mix_PlayChannel(-1, sound[soundIndex], loop);
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

/*
 (void) cleanup attempts to clean up dynamically allocated variables in the
 application, as well as other OpenGL variables created throughout the lifespan
 of the application.

 (void) cleanup does not return any value.
*/
void GameInstance::cleanup() {
    printf("GameInstance::cleanup: Entry\n");
    for (int i = 0; i < controllersConnected; i++) {
        // SDL_GameControllerClose(gameControllers[i]);
    }
    // Cleanup scene objects
    for (auto it = sceneObjects_.begin(); it != sceneObjects_.end(); ++it) {
        destroySceneObject(*it);
    }
    gfxController_->cleanup();
    Mix_CloseAudio();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return;
}
/*
 (int) destroyGameObject takes a (GameObject *) object and attempts to clean up
 all dynamically allocated attributes for that given object.

 On success, 0 is returned and the memory used by that GameObject is freed. On
 failure, -1 is returned and an error is printed to stderr.
 */
int GameInstance::destroySceneObject(SceneObject *object) {
    if (object == NULL) {
        cerr << "Error: Cannot destroy empty GameObject!\n";
        return -1;
    }
    delete object;
    return 0;
}

/*
 (bool) isWindowOpen checks if the current SDL window is currently open still.

 (bool) isWindowOpen returns true when the SDL window is still open, otherwise
 false is returned.
*/
bool GameInstance::isWindowOpen() {
    bool running = true;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || keystate[SDL_SCANCODE_ESCAPE]) {
            cout << "Closing now...\n";
            running = false;
        }
    }
    return running;
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
    std::unique_lock<std::mutex> lock(sceneLock_);
    if (sceneObjects_.empty()) {
        cerr << "Error: No active GameObjects in the current scene!\n";
        return -1;
    }
    gfxController_->update();
    // Update cameras first
    vector<SceneObject *> deferredUpdates;
    for (auto it = sceneObjects_.begin(); it != sceneObjects_.end(); ++it) {
        if ((*it)->type() == ObjectType::CAMERA_OBJECT) (*it)->update();
        else
            deferredUpdates.push_back(*it);
    }
    // Update all other updates after
    for (auto it = deferredUpdates.begin(); it != deferredUpdates.end(); ++it) {
        (*it)->update();
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
    return 0;
}

/*
 (int) setDeltaTime takes a (GLdouble) time and sets the current GameInstance's
 deltaTime value to that time value. This time value should only ever be the
 amount of time in seconds passed since the last rendered frame.

 (int) setDeltaTime returns 0 upon success.
*/
int GameInstance::setDeltaTime(GLdouble time) {
    deltaTime = time;
    return 0;
}

GameObject *GameInstance::createGameObject(Polygon *characterModel, vec3 position, vec3 rotation, GLfloat scale,
    string objectName) {
    printf("GameInstance::createGameObject: Creating GameObject %lu\n", sceneObjects_.size());
    auto gameObject = new GameObject(characterModel, position, rotation, scale, objectName, ObjectType::GAME_OBJECT,
        gfxController_);
    gameObject->setDirectionalLight(directionalLight);
    gameObject->setLuminance(luminance);
    sceneObjects_.push_back(gameObject);
    return gameObject;
}

CameraObject *GameInstance::createCamera(GameObject *target, vec3 offset, GLfloat cameraAngle, GLfloat aspectRatio,
              GLfloat nearClipping, GLfloat farClipping) {
    printf("GameInstance::createCamera: Creating CameraObject %lu\n", sceneObjects_.size());
    auto cameraName = "Camera" + std::to_string(sceneObjects_.size());
    auto gameCamera = new CameraObject(target, offset, cameraAngle, aspectRatio, nearClipping, farClipping,
        ObjectType::CAMERA_OBJECT, cameraName, gfxController_);
    sceneObjects_.push_back(gameCamera);
    return gameCamera;
}

TextObject *GameInstance::createText(string message, string fontPath, GLuint programId, string objectName) {
    printf("GameInstance::createText: Creating TextObject %lu\n", sceneObjects_.size());
    auto text = new TextObject(message, fontPath, programId, objectName, ObjectType::TEXT_OBJECT, gfxController_);
    sceneObjects_.push_back(text);
    return text;
}

SceneObject *GameInstance::getSceneObject(string objectName) {
    SceneObject *result = nullptr;
    for (auto it = sceneObjects_.begin(); it != sceneObjects_.end(); ++it) {
        if ((*it)->getObjectName().compare(objectName) == 0) result = (*it);
    }
    return result;
}

/*
 (GLdouble) getDeltaTime returns the amount of time in second since the last
 rendered frame.
*/
GLdouble GameInstance::getDeltaTime() {
    return deltaTime;
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

/*
 (void) setLuminance sets the current GameInstance's luminance value to the
 provided (GLfloat) luminanceValue.
*/
void GameInstance::setLuminance(GLfloat luminanceValue) {
    luminance = luminanceValue;
}

/*
 (void) initWindow creates a new SDL window instance using the given
 (int) height and (int) width to determine the size of the window in pixels.

 (void) initWindow does not return any values.
*/
void GameInstance::initWindow(int width, int height) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
    window = SDL_CreateWindow("Studious Engine Example", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, width, height,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifdef __APPLE__  // Temporarily restrict SDL AA to MACOS
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, AASAMPLES);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, AASAMPLES);
#endif
    SDL_GLContext mainContext = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(0);  // 0 - Disable VSYNC / 1 - Enable VSYNC
    renderer = SDL_GetRenderer(window);
    if (window == NULL) {
        cerr << "Error: Failed to create SDL window!\n";
        return;
    }
}

/*
 (void) initAudio uses the pathnames provided in the soundList_ vector to
 initialize the audio inside of SDL mixer. In the case of an error during the
 initialization process, initAudio will call exit() with an error code of -1.

 (void) initAudio does not return any values.
*/
void GameInstance::initAudio() {
    audioID = Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 512);
    if (audioID < 0) {
        cerr << "Error: Unable to open audio: " << SDL_GetError() <<
            "\n";
        exit(-1);
    }
    audioID = Mix_AllocateChannels(4);
    if (audioID < 0) {
        cerr << "Error: Unable to allocate mixing channels: "
            << SDL_GetError() << "\n";
        exit(-1);
    }
    vector<string>::iterator it;
    int i = 0;
    for (it = soundList_.begin(); it != soundList_.end(); ++it) {
        sound.push_back(Mix_LoadWAV((*it).c_str()));
        if (sound[i++] == NULL) {
            cerr << "Error: Unable to load wave file: " << soundList_[i] << '\n';
        }
    }
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
