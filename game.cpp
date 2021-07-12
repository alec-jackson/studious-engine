/*
 game.cpp and game.hpp are example files demonstrating the use of the studious
 game engine. These two basic game files will generate a basic scene when the
 engine is compiled and ran.
 */
#include "game.hpp"

/*
 IMPORTANT INFORMATION FOR LOADING SHADERS/SFX:
 Currently, the below global vectors are used for loading in sound effect files,
 texture files and shaders. Adding a new sound to the soundList allows the sound
 to be played by calling the GameInstance::playSound(int soundIndex, int loop)
 method (see documentation for use). When adding a new shader to be used in the
 program, it is IMPORTANT that you pair the vertex shader with the fragment
 shader you want to use together at the same index in the fragShaders and
 vertShaders vectors. For instance, if we had a shader called swamp.vert and
 swamp.frag, we would want both shaders to occur at the same spot in the vector
 (if we already have 2 shader files present, we would add swamp.vert as the
 third element in vertShaders, and swamp.frag as the third in fragShaders). After
 doing this, you should be able to set the programID using the
 GameInstance::getProgramID(int index) method to grab the programID for your
 gameObject. If the shader is in index 2, we would call getProgramID(2) to get
 the appropriate programID. For textures, we specify a path to an image that
 will be opened for a given texture, and specify the textures to use as a
 texture pattern (where each number in the vector corresponds to the index of
 the texture to use).
*/
// Global Variables, should eventually be moved to a config file
vector<string> soundList = {
    "sfx/music/endlessNight.wav"
}; // A list of gameSounds to load
vector<string> fragShaders = {
    "shaders/standardFragment.frag",
    "shaders/coll.frag",
    "shaders/text.frag"
}; // Contains collider renderer and basic object renderer.
vector<string> vertShaders = {
    "shaders/standardVertex.vert",
    "shaders/coll.vert",
    "shaders/text.vert"
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

GameObjectText *fps_counter;

int setup(GameInstance *currentGame, configData* config);
int runtime(GameInstance *currentGame);
int mainLoop(gameInfo *gamein);

int main(int argc, char **argv) {
    int errorNum;
    GameInstance currentGame;
    configData config;
    // Run setup, if fail exit gracefully
    if (setup(&currentGame, &config)){
        exit(1);
    }
    errorNum = runtime(&currentGame);
    return errorNum;
}

/*
 (int) setup takes a (GameInstance *) currentGame instance to use for the game
 scene, and (configData *) config data used for configuring window properties.
 (int) setup configures the resolution of the SDL window.

 (int) setup returns 0 on success.
*/
int setup(GameInstance *currentGame, configData* config){
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
        currentGame->startGameInstance(args);
    }
    return 0;
}

/*
 (int) runtime takes a (GameInstance *) gamein to create the current scene in.
 This function creates all of the GameObjects and GameCameras in the current
 scene and creates a seperate thread for handling user input. All of the setup
 done in the runtime function is for demonstration purposes for now. The final
 studious engine product will source scene information from a .yaml file
 supplied by the user and build the game scene based on that data.

 (int) runtime returns 0 on success.
*/
int runtime(GameInstance *currentGame) {
    cout << "Building game scene!\n";
    SDL_SetRelativeMouseMode(SDL_TRUE);
    struct gameInfo currentGameInfo;
    bool isDone = false;
    cout << "Creating camera.\n";
    vector<int> gameObject(5);
    // Configure a new createCameraInfo struct to pass to createCamera
    // See cameraInfo struct for documentation
    cameraInfo camInfo = { NULL, vec3(5.140022f, 1.349999f, 2.309998f),
        3.14159 / 5.0f, 16.0f / 9.0f, 4.0f, 90.0f };
    gameObject[2] = currentGame->createCamera(camInfo);

    vector<GLint> texturePattern = {0, 1, 2, 3};
    vector<GLint> texturePatternStage = {1};

    cout << "Creating Map.\n";
    //Create an importObj struct for importing the stage
    importObjInfo mapInfo = { "models/testMap1.obj", texturePathStage,
        texturePatternStage, currentGame->getProgramID(0) };

    //Create a gameObjectInfo struct for creating a game object for the map
    gameObjectInfo map = { importObj(mapInfo),
        vec3(-0.006f, -0.019f, 0.0f), vec3(0.0f, 0.0f, 0.0f), 0.009500f,
        gameObject[2], "map" };

    gameObject[0] = currentGame->createGameObject(map);

    cout << "Creating Player\n";

    // Import the player object
    importObjInfo player = { "models/Dracula.obj", texturePath, texturePattern,
        currentGame->getProgramID(0) };

    // Ready the gameObjectInfo for the player object
    gameObjectInfo playerObj = { importObj(player), vec3(0.0f, 0.0f, -1.0f),
        vec3(0.0f, 0.0f, 0.0f), 0.005f, gameObject[2], "player" };

    gameObject[1] = currentGame->createGameObject(playerObj);
    GameObject *dracs = currentGame->getGameObject(gameObject[1]);
    dracs->createCollider(currentGame->getProgramID(1));

    cout << "Creating wolf\n";
    // Import the wold object
    importObjInfo wolf = { "models/wolf.obj", texturePath, texturePattern,
        currentGame->getProgramID(0) };
    // Ready the gameObjectInfo for the wolf object
    gameObjectInfo wolfObj = { importObj(wolf),
        vec3(0.00f, 0.01f, -0.08f), vec3(0.0f, 0.0f, 0.0f), 0.02f,
        gameObject[2], "NPC"};

    gameObject[3] = currentGame->createGameObject(wolfObj);

    GameObject *wolfObject = currentGame->getGameObject(gameObject[3]);
    wolfObject->createCollider(currentGame->getProgramID(1));

    // Configure some in-game text objects
    textObjectInfo textInfo = { "Studious Engine 2021", "misc/fonts/AovelSans.ttf",
        currentGame->getProgramID(2) };
    gameObject[4] = currentGame->createText(textInfo);
    GameObjectText *textObj = currentGame->getText(gameObject[4]);
    textObj->setPos(vec3(25.0f, 25.0f, 0.0f));
    textInfo = { "FPS: ", "misc/fonts/AovelSans.ttf",
        currentGame->getProgramID(2) };
    gameObject[4] = currentGame->createText(textInfo);
    textObj = currentGame->getText(gameObject[4]);
    textObj->setPos(vec3(25.0f, 670.0f, 0.0f));
    fps_counter = textObj;
    fps_counter->setMessage("FPS: 0");
    fps_counter->setScale(0.7f);

    GameCamera *currentCamera = currentGame->getCamera(gameObject[2]);
    currentCamera->setTarget(currentGame->getGameObject(gameObject[1]));
    GameObject *currentGameObject = currentGame->getGameObject(gameObject[1]);
    currentGameObject->setRotation(vec3(0, 0, 0));
    currentGameObject = currentGame->getGameObject(gameObject[3]);
    currentGameObject = currentGame->getGameObject(gameObject[1]);
    cout << "currentGameObject tag is " << currentGameObject->getCollider()
        << '\n';

    currentGameObject->setPos(vec3(-0.005f, 0.01f, 0.0f));
    currentGameObject->setRotation(vec3(0.0f, 180.0f, 0.0f));
    currentGameObject->setScale(0.0062f);

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
 (int) mainLoop starts rendering objects in the current GameInstance to the
 main SDL window. The methods called from the currentGame object render parts
 of the scene.

 (int) mainLoop returns 0 when closed successfully. When an error occurs and the
 mainLoop closes prematurely, an error code is returned.
*/
int mainLoop(gameInfo* gamein) {
    Uint64 begin, end;
    int running = 1;
    double currentTime = 0.0, sampleTime = 1.0;
    GameInstance *currentGame = gamein->currentGame;
    double deltaTime;
    short error = 0;
    vector<double> times;
    while (running) {
        begin = SDL_GetPerformanceCounter();
        running = currentGame->isWindowOpen();
        currentGame->updateOGL();
        error = currentGame->updateCameras();
        error |= currentGame->updateObjects();
        error |= currentGame->updateWindow();
        if (error) {
            return error;
        }
        end = SDL_GetPerformanceCounter();
        deltaTime = (double)(end - begin) / (SDL_GetPerformanceFrequency());
        currentGame->setDeltaTime(deltaTime);
        if (SHOW_FPS) { // use sampleSize to find average FPS
            times.push_back(deltaTime);
            currentTime += deltaTime;
            if (currentTime > sampleTime) {
                currentTime = 0.0f;
                double sum = 0.0;
                vector<double>::iterator it;
                for (it = times.begin(); it != times.end(); ++it) sum += *it;
                sum /= times.size();
                times.clear();
                cout << "FPS: " << 1.0 / sum << '\n';
                fps_counter->setMessage("FPS: " + to_string(int(1.0 / sum)));
            }
        }
    }
    return 0;
}
