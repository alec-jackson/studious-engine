/*
 game.cpp and game.hpp are example files demonstrating the use of the studious
 game engine. These two basic game files will generate a basic scene when the
 engine is compiled and ran.
 */
#include "game.hpp"

// Global Variables, should eventually be moved to a config file
vector<string> soundList = {
    "sfx/music/endlessNight.wav"
}; // A list of gameSounds to load
vector<string> fragShaders = {
    "shaders/standardFragment.frag",
    "shaders/coll.frag",
    "shaders/text.fragmentshader"
}; // Contains collider renderer and basic object renderer.
vector<string> vertShaders = {
    "shaders/standardVertex.vert",
    "shaders/coll.vert",
    "shaders/text.vertexshader"
}; // Contains collider renderer and basic object renderer.
vector<string> texturePathStage = {
    "images/skintexture.jpg"
};
vector<string> texturePath = {
    "images/rock_texture.jpg",
    "images/rock_texture.jpg",
    "images/shoetexture.jpg",
    "images/shirttexture.jpg"
};

int setup(GameInstance *currentGame, ConfigData* config);
int runtime(GameInstance *currentGame);
int mainLoop(gameInfo *gamein);

int main(int argc, char **argv) {
    int errorNum;
    GameInstance currentGame;
    ConfigData config;
    // Run setup, if fail exit gracefully
    if (setup(&currentGame, &config)){
        exit(1);
    }
    errorNum = runtime(&currentGame);
    return errorNum;
}

/*
 (int) setup takes a (mutex *)infoLock for locking elements in the scene,
 the (GameInstance *) currentGame instance to use for the game scene, and
 (ConfigData *) config data used for configuring window properties. (int) setup
 configures the resolution of the SDL window. On success, 0 is returned.
*/
int setup(GameInstance *currentGame, ConfigData* config){
    int flag = loadConfig(config, "misc/config.txt");
    gameInstanceArgs args;
    args.soundList = soundList;
    args.vertexShaders = vertShaders;
    args.fragmentShaders = fragShaders;
    if (!flag) {
        args.windowWidth = config->resX;
        args.windowHeight = config->resY;
        currentGame -> startGameInstance(args);
    } else {
        args.windowWidth = 1280;
        args.windowHeight = 720;
        currentGame -> startGameInstance(args);
    }
    return 0;
}

/*
 (int) runtime takes a (mutex *) infoLock for locking shared resources between
 threads, and a (GameInstance *) gamein to create the current scene in. This
 function creates all of the GameObjects and GameCameras in the current scene
 and creates a seperate thread for handling user input. All of the setup done in
 the runtime function is for demonstration purposes for now. The final studious
 engine product will source scene information from a .yaml file supplied by the
 user and build the game scene based on that data. On success, 0 is returned.
*/
int runtime(GameInstance *currentGame) {
    cout << "Building game scene!\n";
    SDL_SetRelativeMouseMode(SDL_TRUE);
    struct gameInfo currentGameInfo;
    bool isDone = false;
    mutex sceneLock; // Lock used for locking scene elements

    cout << "Creating camera.\n";
    int gameObject[4];
    // Configure a new createCameraInfo struct to pass to createCamera
    cameraInfo camInfo;
    camInfo.objTarget = NULL;
    camInfo.offset = vec3(5.140022f, 1.349999f, 2.309998f);
    camInfo.viewCameraAngle = 3.14159/5.0f;
    camInfo.viewAspectRatio = 16.0f / 9.0f; // 16:9 Ratio
    camInfo.viewNearClipping = 4.0f;
    camInfo.viewFarClipping = 90.0f;
    gameObject[2] = currentGame->createCamera(camInfo);

    vector<GLint> texturePattern = {0, 1, 2, 3};
    vector<GLint> texturePatternStage = {1};

    cout << "Creating Map.\n";
    //Create an importObj struct for importing the stage
    importObjInfo mapInfo;
    mapInfo.modelPath = "models/testMap1.obj";
    mapInfo.texturePath = texturePathStage;
    mapInfo.texturePattern = texturePatternStage;
    mapInfo.programID = currentGame->getProgramID(0);
    //Create a gameObjectInfo struct for creating a game object for the map
    gameObjectInfo map;
    map.characterModel = importObj(mapInfo);
    map.scaleVec = vec3(0.009500f, 0.009500f, 0.009500f);
    map.pos = vec3(-0.006f, -0.019f, 0.0f);
    map.rotateAxis = vec3(0.0f, 0.0f, 1.0f);
    map.rotAngle = 0.0f;
    map.camera = gameObject[2];
    map.collisionTagName = "map";
    map.colliderObject = NULL;
    gameObject[0] = currentGame->createGameObject(map);

    cout << "Creating Player\n";
    // The collider on the object is just a basic wire frame at the moment
    // Configure the wireframe box around the player
    importObjInfo humColInfo;
    humColInfo.modelPath = "models/rockStone.obj";
    humColInfo.programID = currentGame->getProgramID(1);

    polygon *humanCollider = importObj(humColInfo);

    // Import the player object
    importObjInfo player;
    //player.modelPath = "models/tank.obj";
    player.modelPath = "models/sphere.obj";
    player.texturePath = texturePath;
    player.texturePattern = texturePattern;
    player.programID = currentGame->getProgramID(0);

    // Ready the gameObjectInfo for the player object
    gameObjectInfo playerObj;
    playerObj.characterModel = importObj(player);
    playerObj.scaleVec = vec3(0.005f, 0.005f, 0.005f); // Arbitrary hell
    playerObj.pos = vec3(0.0f, 0.0f, -1.0f);
    playerObj.rotateAxis = vec3(0.0f, 0.0f, 1.0f);
    playerObj.rotAngle = 0.0f;
    playerObj.camera = gameObject[2];
    playerObj.collisionTagName = "player";
    playerObj.colliderObject = 0; //humanCollider;

    gameObject[1] = currentGame->createGameObject(playerObj);

    cout << "Creating wolf\n";
    // Import the wold object
    importObjInfo wolf;
    wolf.modelPath = "models/wolf.obj";
    wolf.texturePath = texturePath;
    wolf.texturePattern = texturePattern;
    wolf.programID = currentGame->getProgramID(0);
    // Ready the gameObjectInfo for the wolf object
    gameObjectInfo wolfObj;
    wolfObj.characterModel = importObj(wolf);
    wolfObj.scaleVec = vec3(0.02f, 0.02f, 0.02f); // Arbitrary hell
    wolfObj.pos = vec3(0.00f, 0.01f, -0.08f);
    wolfObj.rotateAxis = vec3(0.0f, 0.0f, 1.0f);
    wolfObj.rotAngle = 0.0f;
    wolfObj.camera = gameObject[2];
    wolfObj.collisionTagName = "NPC";
    wolfObj.colliderObject = humanCollider;

    gameObject[3] = currentGame->createGameObject(wolfObj);

    GameCamera *currentCamera = currentGame->getCamera(gameObject[2]);
    currentCamera->setTarget(currentGame->getGameObject(gameObject[1]));
    GameObject *currentGameObject = currentGame->getGameObject(gameObject[1]);
    currentGameObject->rotateObject(vec3(0, 0, 0));
    currentGameObject = currentGame->getGameObject(gameObject[3]);
    GLfloat wolfScale = 0.01f;

    currentGameObject = currentGame->getGameObject(gameObject[1]);
    cout << "currentGameObject tag is " << currentGameObject->getCollider()
        << '\n';

    currentGameObject->sendPosition(vec3(-0.005f, 0.01f, 0.0f));
    currentGameObject->rotateObject(vec3(0.0f, 180.0f, 0.0f));
    currentGameObject->sendScale(0.0062f);

    currentGameInfo.isDone = &isDone;
    currentGameInfo.gameCamera = currentCamera;
    currentGameInfo.currentGame = currentGame;
    /*
     End Scene Loading
     */

    // Additional threads should be added, pipes will most likely be required
    // Might also be a good idea to keep the parent thread local to watch for
    // unexpected failures and messages from children
    thread rotThread(rotateShape, &currentGameInfo, currentGameObject);
    mainLoop(&currentGameInfo);
    isDone = true;
    rotThread.join();
    cout << "Running cleanup\n";
    currentGame->cleanup();
    return 0;
}

/*
 (void) mainLoop starts rendering objects in the current GameInstance to the
 main SDL window, locking variables in the scene using the (mutex *)sceneLock
 argument to prevent race conditions.

 (void) mainLoop does not return a value.
*/
int mainLoop(gameInfo* gamein) {
    clock_t begin, end; // Used for measuring FPS
    int running = 1, sampleSize = 1000;
    GameInstance *currentGame = gamein->currentGame;
    GLdouble deltaTime;
    short error = 0;
    vector<double> times;
    while (running) {
        running = currentGame->isWindowOpen();
        begin = clock();
        currentGame->updateOGL();
        error = currentGame->updateCameras();
        error |= currentGame->updateObjects();
        error |= currentGame->updateWindow();
        if (error){
            return 1;
        }
        end = clock();
        deltaTime = (double)(end - begin) / (double)CLOCKS_PER_SEC;
        currentGame->setDeltaTime(deltaTime);
        if (SHOW_FPS) { // use sampleSize to find average FPS
            times.push_back(deltaTime);
            if (times.size() > sampleSize) {
                double sum = 0.0;
                vector<double>::iterator it;
                for (it = times.begin(); it != times.end(); ++it) sum += *it;
                sum /= times.size();
                times.clear();
                cout << "FPS :" << 1.0 / sum << '\n';
            }
        }
    }
    return 0;
}
