#include "gameInstance.hpp"

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
void GameInstance::startGameInstance(gameInstanceArgs args) {
    sfxNames = args.soundList;
    width = args.windowWidth;
    height = args.windowHeight;
    luminance = 1.0f; // Set default values
    directionalLight = vec3(-100, 100, 100);
    controllersConnected = 0;
    initWindow(width, height);
    initAudio();
    playSound(0, 1);
    initController();
    initApplication(args.vertexShaders, args.fragmentShaders);
    keystate = SDL_GetKeyboardState(NULL);
    gameCameraCount = 0;
    gameObjectCount = 0;
    gameObjects = NULL;
    gameCameras = NULL;
    text.initText();
}

/*
 (int) getWidth returns the current width of the single window for the current
 GameInstance.
 */
int GameInstance::getWidth() {
    return width;
}

/*
 (int) getHeight returns the current height of the single window for the current
 GameInstance.
 */
int GameInstance::getHeight() {
    return height;
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
 (GLuint) getProgramID returns the programID associated with the given
 (int) index.

 On success, function returns associated programID. On failure,
 -1 is returned and an error is written to stderr.
*/
GLuint GameInstance::getProgramID(int index) {
    if (index < 0 || index >= programID.size()) {
        cerr << "Error: Requested programID is not in available range [0, "
        << programID.size() - 1 << "]\n";
    }
    return programID[index];
}

/*
 (controllerReadout *) getControllers takes an (int) controllerIndex and returns
 the associated controllerReadout struct.
*/
controllerReadout* GameInstance::getControllers(int controllerIndex){
    controllerInfo[controllerIndex].leftAxis = SDL_GameControllerGetAxis(gameControllers[controllerIndex], SDL_CONTROLLER_AXIS_LEFTY );
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
void GameInstance::changeWindowMode(int mode){
    SDL_DisplayMode DM;
    // if(mode > 1){
    //     SDL_GetCurrentDisplayMode(0, &DM);
    // } else {
    //     DM.w = width;
    //     DM.h = height;
    // }
    //SDL_SetWindowSize(window, DM.w, DM.h);
    SDL_SetWindowFullscreen(window, mode);
    //SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

/*
 (void) cleanup attempts to clean up dynamically allocated variables in the
 application, as well as other OpenGL variables created throughout the lifespan
 of the application.

 (void) cleanup does not return any value.
*/
void GameInstance::cleanup() {
    for (int i = 0; i < controllersConnected; i++) {
        //SDL_GameControllerClose(gameControllers[i]);
    }
    gameObjectLL *currentGameObject = gameObjects;
    gameObjectLL *temp;
    for (int i = 0; i < gameObjectCount; i++) {
        temp = currentGameObject;
        currentGameObject = currentGameObject->next;
        destroyGameObject(temp->current);
        delete temp;
    }
    vector<GLuint>::iterator it;
    for (it = programID.begin(); it != programID.end(); ++it) {
        glDeleteProgram(*it);
    }
    glDeleteVertexArrays(1, &vertexArrayID);
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
    // Delete OpenGL buffers for the gameObjects
    for (int i = 0; i < object->getModel()->numberOfObjects; i++) {
        glDeleteBuffers(1, &object->getModel()->shapebufferID[i]);
        glDeleteBuffers(1, &object->getModel()->textureCoordsID[i]);
        glDeleteBuffers(1, &object->getModel()->normalbufferID[i]);
        glDeleteTextures(1, &object->getModel()->textureID[i]);
    }
    delete object->getModel();
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
    while(SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || keystate[SDL_SCANCODE_ESCAPE]) {
            cout << "Closing now...\n";
            running = false;
        }
    }
    return running;
}

/*
 (void) updateOGL clears the current OpenGL drawing on the SDL window. This
 function should be called after every frame.

 (void) updateOGL does not return any value.
*/
void GameInstance::updateOGL(){
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(0.0f, 0.0f, 0.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/*
 (int) updateCameras loops through all of the cameras present in the current
 game scene and updates their position matrices.

 On success, 0 is returned and the cameras in the game scene are updated. On
 failure, 1 is returned and the cameras in the game scene are not updated.
*/
int GameInstance::updateCameras() {
    gameCameraLL *currentGameCamera = gameCameras;
    if (currentGameCamera == NULL) {
        cerr << "Error: No cameras found in active scene!\n";
        return 1;
    }
    // Update the VP matrix for each camera
    for (int i = 0; i < gameCameraCount; i++) {
        currentGameCamera->current->updateCamera();
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

 On success, 0 is returned and the GameInstance scene is rendered. On failure, 1
 is returned and nothing is rendered.
*/
int GameInstance::updateObjects() {
    if (gameObjects == NULL) {
        cerr << "Error: No active GameObjects in the current scene!\n";
    }
    gameObjectLL *currentGameObject = gameObjects;
    for (int i = 0; i < gameObjectCount; i++) {
        currentGameObject->current->setDirectionalLight(directionalLight);
        currentGameObject->current->setLuminance(luminance);
        // Send the new VP matrix to the current gameObject being drawn.
        currentGameObject->current->setVPMatrix(getCamera(currentGameObject->current->getCameraID())->getVPMatrix());
        currentGameObject->current->drawShape();
        currentGameObject = currentGameObject->next;
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
int GameInstance::setDeltaTime(GLdouble time){
    deltaTime = time;
    return 0;
}

/*
 (int) createGameObject takes some (gameObjectInfo) objectInfo and constructs a
 new GameObject into the scene using information in the objectInfo struct. The
 new GameObject that is created is allocated in the heap, so it will need a
 delete statement in the cleanup function. When a new GameObject is created, it
 is put onto the (gameObjectLL) gameObjects linked list inside of the
 GameInstance and assigned an ID number. The ID number assigned to a GameObject
 in the scene corresponds to whatever the gameObjectCount was at the time of
 creation (hence why the first gameobject created will have an ID of 0).

 (int) createGameObject returns the index of the newly created GameObject inside
 of the current GameInstance on success. On failure, -1 is returned and the
 GameObject is not created.
*/
int GameInstance::createGameObject(gameObjectInfo objectInfo) {
    cout << "Creating GameObject " << gameObjectCount << "\n";
    // If the gameObjects array is empty
    if (!gameObjectCount) {
        gameObjects = new gameObjectLL;
        gameObjects->current = new GameObject;
        gameObjects->current->configureGameObject(objectInfo);
        gameObjectCount = 1;
        gameObjects->next = NULL; // Set end of linked list
        return 0;
    } else {
        int currentIndex = 1;
        gameObjectLL *currentGameObject = gameObjects;
        while (currentGameObject->next != NULL) {
            currentGameObject = currentGameObject->next;
            currentIndex++;
        }
        currentGameObject->next = new gameObjectLL;
        currentGameObject = currentGameObject->next;
        currentGameObject->current = new GameObject;
        currentGameObject->current->configureGameObject(objectInfo);
        gameObjectCount++;
        currentGameObject->next = NULL;
        return currentIndex;
    }
}

/*
 (int) createCamera takes some (cameraInfo) camInfo describing settings to use
 for a new camera, and creates a new camera in a similar manner to how standard
 GameObjects are created (see notes above for details on GameObject creation
 and functionality of GameObject Linked Lists).

 On success, the ID of the created GameCamera is returned. On failure, the
 GameCamera is not created and -1 is returned.
*/
int GameInstance::createCamera(cameraInfo camInfo) {
    cout << "Creating GameCamera " << gameCameraCount << "\n";
    // If the gameObjects array is empty
    if (!gameCameraCount) {
        gameCameras = new gameCameraLL;
        gameCameras->current = new GameCamera;
        gameCameras->current->configureCamera(camInfo);
        gameCameraCount = 1;
        gameCameras->next = NULL;
        return 0;
    } else {
        int currentIndex = 1;
        gameCameraLL *currentGameCamera = gameCameras;
        while (currentGameCamera->next != NULL) {
            currentGameCamera = currentGameCamera->next;
            currentIndex++;
        }
        currentGameCamera->next = new gameCameraLL;
        currentGameCamera = currentGameCamera->next;
        currentGameCamera->current = new GameCamera;
        currentGameCamera->current->configureCamera(camInfo);
        gameCameraCount++;
        currentGameCamera->next = NULL;
        return currentIndex;
    }
}

/*
 (GameObject *) getGameObject walks through the (gameObjectLL *) gameObjects
 linked list in the current GameInstance and returns the GameObject with the
 matching (int) gameObjectID.

 On success, a pointer to the discovered GameObject is returned. On failure,
 NULL is returned.
 */
GameObject * GameInstance::getGameObject(int gameObjectID) {
    if (gameObjectID > gameObjectCount - 1) {
        cerr << "Error: GameObject with gameObjectID " << gameObjectID <<
            " does not exist in teh current context!\n";
        return NULL;
    }
    gameObjectLL *currentGameObject = gameObjects;
    for (int i = 0; i < gameObjectID; i++) {
        currentGameObject = currentGameObject->next;
    }
    return currentGameObject->current;
}

/*
 (GameCamera *) getCamera walks through the (gameCameraLL *) in the current
 GameInstance and returns the GameCamera in the game scene with the ID that
 matches (int) gameCameraID.

 On success, a pointer to the matching GameCamera is returned. On failure, NULL
 is returned.
*/
GameCamera *GameInstance::getCamera(int gameCameraID) {
    if (gameCameraID > gameCameraCount - 1) {
        cerr << "Error: GameCamera with gameCameraID " << gameCameraID <<
            " does not exist in teh current context!\n";
        return NULL;
    }
    gameCameraLL *currentGameCamera = gameCameras;
    for (int i = 0; i < gameCameraID; i++) {
        currentGameCamera = currentGameCamera->next;
    }
    return currentGameCamera->current;
}

/*
 (GLdouble) getDeltaTime returns the amount of time in second since the last
 rendered frame.
*/
GLdouble GameInstance::getDeltaTime() {
    return deltaTime;
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
    window = SDL_CreateWindow("OGL Engine Beta", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, width, height,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GLContext mainContext = SDL_GL_CreateContext(window);
    SDL_GL_SetSwapInterval(0); // 0 - Disable VSYNC / 1 - Enable VSYNC
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
 (void) initAudio uses the pathnames provided in the sfxNames vector to
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
    for (it = sfxNames.begin(); it != sfxNames.end(); ++it) {
        sound.push_back(Mix_LoadWAV((*it).c_str()));
        if (sound[i++] == NULL) {
            cerr << "Error: Unable to load wave file: " << sfxNames[i] << '\n';
        }
    }
}

/*
 (void) initController attemps to initialize any connected joysticks with the
 current SDL instance.

 (void) initController does not return any values.
*/
void GameInstance::initController(){
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
    // Compile each of our shaders and assign them their own programID number
    GLuint vertexArrayID;
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);
    for (int i = 0; i < vertexPath.size(); i++) {
        programID.push_back(LoadShaders(vertexPath[i].c_str(), fragmentPath[i].c_str()));
    }
}

/*
 (void) basicCollision takes a (GameInstance *) gameInstance and performs a
 basic collision check on all of the active GameObjects in the scene. This
 method is still a WIP and does not really do anything at the moment.

 (void) basicCollision does not return any values.
*/
void GameInstance::basicCollision(GameInstance *gameInstance) {
    gameObjectLL *objList = gameInstance->gameObjects;
    GameObject *curObj;
    bool runFlag = true;
    if (objList->current != 0) {
        do {
            curObj = objList->current;
        } while (runFlag);
    }
}
