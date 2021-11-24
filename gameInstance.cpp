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
 UINT_MAX is returned and an error is written to stderr.
*/
GLuint GameInstance::getProgramID(uint index) {
    if (index >= programID.size()) {
        cerr << "Error: Requested programID is not in available range [0, "
        << programID.size() - 1 << "]\n";
        return UINT_MAX;
    }
    return programID[index];
}

/*
 (controllerReadout *) getControllers takes an (int) controllerIndex and returns
 the associated controllerReadout struct.
*/
controllerReadout* GameInstance::getControllers(int controllerIndex){
    controllerInfo[controllerIndex].leftAxis =
        SDL_GameControllerGetAxis(gameControllers[controllerIndex],
        SDL_CONTROLLER_AXIS_LEFTY );
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
    //SDL_DisplayMode DM;
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
    vector<GameObject *>::iterator git;
    for (git = gameObjects.begin(); git != gameObjects.end(); ++git) {
        destroyGameObject((*git));
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
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCullFace(GL_BACK);
    glDepthFunc(GL_LESS);
    glClearColor(0.0f, 0.0f, 0.0, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

/*
 (int) updateCameras loops through all of the cameras present in the current
 game scene and updates their position matrices.

 On success, 0 is returned and the cameras in the game scene are updated. On
 failure, -1 is returned and the cameras in the game scene are not updated.
*/
int GameInstance::updateCameras() {
    if (gameCameras.empty()) {
        //cerr << "Error: No cameras found in active scene!\n";
        return -1;
    }
    vector<GameCamera *>::iterator it;
    // Update the VP matrix for each camera
    for (it = gameCameras.begin(); it != gameCameras.end(); ++it) {
        (*it)->updateCamera();
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
    glBindVertexArray(vertexArrayID);
    std::vector<GameObject *>::iterator it;
    for (it = gameObjects.begin(); it != gameObjects.end(); ++it) {
        (*it)->setDirectionalLight(directionalLight);
        (*it)->setLuminance(luminance);
        // Send the new VP matrix to the current gameObject being drawn.
        (*it)->setVPMatrix(getCamera((*it)->getCameraID())->getVPMatrix());
        (*it)->drawShape();
    }
    // If there is any active texts in the scene, render them
    std::vector<GameObjectText *>::iterator text_it;
    for (text_it = gameTexts.begin(); text_it != gameTexts.end(); ++text_it) {
        (*text_it)->drawText();
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
 is put into the (vector<GameObject>) gameObjects vector and rendered in the
 updateObjects method.

 (int) createGameObject returns the index of the newly created GameObject inside
 of the current GameInstance on success. On failure, -1 is returned and the
 GameObject is not created.
*/
int GameInstance::createGameObject(gameObjectInfo objectInfo) {
    cout << "Creating GameObject " << gameObjects.size() << "\n";
    GameObject *gameObject = new GameObject();
    gameObject->configureGameObject(objectInfo);
    gameObjects.push_back(gameObject);
    return gameObjects.size() - 1;
}

/*
 (int) createCamera takes some (cameraInfo) camInfo describing settings to use
 for a new camera, and creates a new camera in a similar manner to how standard
 GameObjects are created (see notes above for details on GameObject creation).

 (int) createCamera returns the ID of the created GameCamera on success. On
 failure, the GameCamera is not created and -1 is returned.
*/
int GameInstance::createCamera(cameraInfo camInfo) {
    cout << "Creating GameCamera " << gameCameras.size() << "\n";
    GameCamera *gameCamera = new GameCamera();
    gameCamera->configureCamera(camInfo);
    gameCameras.push_back(gameCamera);
    return gameCameras.size() - 1;
}

/*
 (int) createText takes an (textObjectInfo) info argument and creates a new
 GameObjectText GameObject inside of the current GameInstance.

 (int) createText returns the index of the created GameObjectText in the current
 GameInstance.
*/
int GameInstance::createText(textObjectInfo info) {
    cout << "Creating Gametext " << gameTexts.size() << "\n";
    GameObjectText *text = new GameObjectText();
    text->initializeText(info);
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
 (GameCamera *) getGameCamera returns the GameCamera pointer for the GameCamera
 located at the index gameCameraID inside of the (vector<GameCamera *>)
 gameCameras vector on success. On failure, NULL is returned and an error is
 printed to stderr.
*/
GameCamera *GameInstance::getCamera(uint gameCameraID) {
    if (!gameCameras.size() || gameCameras.size() < gameCameraID) {
        cerr << "Error: getCamera index out of bounds!\n";
        return NULL;
    }
    return gameCameras[gameCameraID];
}

/*
 (GameObjectText *) getText returns the GameObjectText pointer for the
 GameObjectText located at the index gameTextID inside of the
 (vector<GameObjectText *>) gameTexts vector on success. On failure, NULL is
 returned and an error is printed to stderr.
*/
GameObjectText *GameInstance::getText(uint gameTextID) {
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


void printVector(vec4 item) {
    cout << "Item: " << item.x << " " << item.y << " " << item.z << " " << item.w << endl;
}

/* [OUTDATED]
 (int) getCollision takes a (GameObject *) object1 and compares it to
 (GameObject *) object2 and checks whether the two GameObjects are either
 colliding or about to collide. getCollision checks whether the two GameObjects
 are about to collide using the (vec3) moving parameter. The general formula for
 this function is thefollowing:
 1. A collision is only currently occuring if object1's xDist, yDist and zDist
    are in range of the center of object2's center + (xDist, yDist, zDist).
 2. A collision is about to occur if object1's (xDist, yDist, zDist) +
    (vec3) moving is in range of the center of object2's
    center + (xDist, yDist, zDist).

 On success, (int) getCollision will return 1 if the two GameObjects are
 currently colliding, 2 if the two objects are about to collide, or 0 if there
 is no collision. Otherwise, -1 is returned.
*/
int GameInstance::getCollision(GameObject *object1, GameObject *object2,
    vec3 moving) {
    if (object1 == NULL || object2 == NULL) {
        cerr << "Error: Cannot get collision for NULL GameObjects!\n";
        return -1;
    }
    // Obtain lock and update positions
    object1->lockObject();
    object2->lockObject();
    int matching = 0; // Number of axis that have collided
    colliderInfo obj1collider = object1->getCollider();
    colliderInfo obj2collider = object2->getCollider();
    vec4 center1, center2;
    vec4 offset1, offset2;
    center1 = obj1collider.center;
    center2 = obj2collider.center;
    //printVector(center1);
    //printVector(center2);
    offset1 = obj1collider.offset;
    offset2 = obj2collider.offset;
    /*
    cout << "Object 1 Details" << endl;
    printVector(center1);
    printVector(vec4(offset1, 0));
    cout << "Object 2 Details" << endl;
    printVector(center2);
    printVector(vec4(offset2, 0));
    cout << "Actual position" << endl;
    printVector(vec4(object1->getPos(), 0));
    */
    object1->unlockObject();
    object2->unlockObject();

    cout << "Center1: " << center1.x << " " << center1.y << " " << center1.z << endl;
    cout << "Center2: " << center2.x << " " << center2.y << " " << center2.z << endl;
    cout << "Offset1: " << offset1.x << " " << offset1.y << " " << offset1.z << endl;
    cout << "Offset2: " << offset2.x << " " << offset2.y << " " << offset2.z << endl;
    
    // First check if the two objects are currently colliding
    for (int i = 0; i < 3; i++) {
        float delta = abs(center2[i] - center1[i]);
        float range = offset1[i] + offset2[i];
        //cout << "distance [ " << i << "] = " << delta - range << endl;
        if (range >= delta) {
            matching++;
        }
    }
    // Return if the objects are currently colliding
    if (matching == 3) return 1;
    matching = 0;
    for (int i = 0; i < 3; i++) {
        float delta = abs(center2[i] - abs(center1[i] + moving[i]));
        float range = offset1[i] + offset2[i];
        if (range >= delta) {
            matching++;
        }
    }
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
    // Compile each of our shaders and assign them their own programID number
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);
    for (uint i = 0; i < vertexPath.size(); i++) {
        programID.push_back(loadShaders(vertexPath[i].c_str(), fragmentPath[i].c_str()));
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
