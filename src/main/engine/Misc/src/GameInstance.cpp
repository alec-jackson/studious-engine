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
    for (int i = 0; i < controllersConnected; i++) {
        // SDL_GameControllerClose(gameControllers[i]);
    }
    vector<GameObject *>::iterator git;
    for (git = gameObjects.begin(); git != gameObjects.end(); ++git) {
        destroyGameObject((*git));
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
int GameInstance::destroyGameObject(GameObject *object) {
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
 (int) updateCameras loops through all of the cameras present in the current
 game scene and updates their position matrices.

 On success, 0 is returned and the cameras in the game scene are updated. On
 failure, -1 is returned and the cameras in the game scene are not updated.
*/
int GameInstance::updateCameras() {
    if (gameCameras.empty()) {
        // cerr << "Error: No cameras found in active scene!\n";
        return -1;
    }
    vector<CameraObject *>::iterator it;
    // Update the VP matrix for each camera
    for (it = gameCameras.begin(); it != gameCameras.end(); ++it) {
        (*it)->render();
    }
    return 0;
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
    if (gameObjects.empty()) {
        cerr << "Error: No active GameObjects in the current scene!\n";
        return -1;
    }
    gfxController_->update();
    /// @todo: Polymorphism! Only need one structure for these game objects
    std::vector<GameObject *>::iterator it;
    for (it = gameObjects.begin(); it != gameObjects.end(); ++it) {
        (*it)->setDirectionalLight(directionalLight);
        (*it)->setLuminance(luminance);
        // Send the new VP matrix to the current gameObject being drawn.
        (*it)->setVpMatrix(getCamera((*it)->getCameraId())->getVpMatrix());
        (*it)->render();
    }
    // If there is any active texts in the scene, render them
    std::vector<TextObject *>::iterator text_it;
    for (text_it = gameTexts.begin(); text_it != gameTexts.end(); ++text_it) {
        (*text_it)->render();
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

/*
 (int) createGameObject takes some (gameObjectInfo) objectInfo and constructs a
 new GameObject into the scene using information in the objectInfo struct. The
 new GameObject that is created is allocated in the heap, so it will need a
 delete statement in the cleanup function. When a new GameObject is created, it
 is put into the (vector<GameObject>) gameObjects vector and rendered in the
 updateObjects method.

 (int) createGameObject returns the index of the newly created GameObject inside
 of the current GameInstance on success. On failure, -1 is returned and the
 GameObject is not created.
*/
int GameInstance::createGameObject(Polygon *characterModel, vec3 position, vec3 rotation, GLfloat scale, int camera,
    string objectName) {
    cout << "Creating GameObject " << gameObjects.size() << "\n";
    GameObject *gameObject = new GameObject(characterModel, position, rotation, scale, camera, objectName,
        gfxController_);
    gameObjects.push_back(gameObject);
    return gameObjects.size() - 1;
}

/*
 (int) createCamera takes some (cameraInfo) camInfo describing settings to use
 for a new camera, and creates a new camera in a similar manner to how standard
 GameObjects are created (see notes above for details on GameObject creation).

 (int) createCamera returns the ID of the created CameraObject on success. On
 failure, the CameraObject is not created and -1 is returned.
*/
int GameInstance::createCamera(GameObject *target, vec3 offset, GLfloat cameraAngle, GLfloat aspectRatio,
              GLfloat nearClipping, GLfloat farClipping) {
    cout << "Creating CameraObject " << gameCameras.size() << "\n";
    auto gameCamera = new CameraObject(target, offset, cameraAngle, aspectRatio, nearClipping, farClipping,
        gfxController_);
    gameCameras.push_back(gameCamera);
    return gameCameras.size() - 1;
}

/*
 (int) createText takes an (textObjectInfo) info argument and creates a new
 TextObject GameObject inside of the current GameInstance.

 (int) createText returns the index of the created TextObject in the current
 GameInstance.
*/
int GameInstance::createText(textObjectInfo info) {
    cout << "Creating Gametext " << gameTexts.size() << "\n";
    TextObject *text = new TextObject(info);
    gameTexts.push_back(text);
    return gameTexts.size() - 1;
}

/*
 (GameObject *) getGameObject returns the GameObject pointer for the GameObject
 located at the index gameObjectID inside of the (vector<GameObject *>)
 gameObjects vector on success. On failure, NULL is returned and an error is
 printed to stderr.
 */
GameObject *GameInstance::getGameObject(uint gameObjectID) {
    if (!gameObjects.size() || gameObjects.size() < gameObjectID) {
        cerr << "Error: getGameObject index out of bounds!\n";
        return NULL;
    }
    return gameObjects[gameObjectID];
}

/*
 (CameraObject *) getCameraObject returns the CameraObject pointer for the CameraObject
 located at the index gameCameraID inside of the (vector<CameraObject *>)
 gameCameras vector on success. On failure, NULL is returned and an error is
 printed to stderr.
*/
CameraObject *GameInstance::getCamera(uint gameCameraID) {
    if (!gameCameras.size() || gameCameras.size() < gameCameraID) {
        cerr << "Error: getCamera index out of bounds!\n";
        return NULL;
    }
    return gameCameras[gameCameraID];
}

/*
 (TextObject *) getText returns the TextObject pointer for the
 TextObject located at the index gameTextID inside of the
 (vector<TextObject *>) gameTexts vector on success. On failure, NULL is
 returned and an error is printed to stderr.
*/
TextObject *GameInstance::getText(uint gameTextID) {
    if (!gameTexts.size() || gameTexts.size() < gameTextID) {
        cerr << "Error: getText index out of bounds!\n";
        return NULL;
    }
    return gameTexts[gameTextID];
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
    int matching = 0;  // Number of axis that have collided
    if (object1 == NULL || object2 == NULL) {
        cerr << "Error: Cannot get collision for NULL GameObjects!\n";
        return -1;
    }

    auto &coll1 = object1->getCollider();
    auto &coll2 = object2->getCollider();

    // First check if the two objects are currently colliding
    for (int i = 0; i < 3; i++) {
        float delta = abs(coll2.center[i] - coll1.center[i]);
        float range = coll1.offset[i] + coll2.offset[i];
        if (range >= delta) {
            matching++;
        }
    }
    // Return if the objects are currently colliding
    if (matching == 3) return 1;
    matching = 0;
    for (int i = 0; i < 3; i++) {
        float delta = abs(coll2.center[i] - coll1.center[i] + moving[i]);
        float range = coll1.offset[i] + coll2.offset[i];
        if (range >= delta) {
            matching++;
        }
    }
    // Return if the objects are about to collide
    if (matching == 3) return 2;
    return 0;
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
    if (glewInit() != GLEW_OK) {
        cerr << "Error: Failed to initialize GLEW!\n";
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
    sceneLock.lock();
    return 0;
}

int GameInstance::unlockScene() {
    sceneLock.unlock();
    return 0;
}
