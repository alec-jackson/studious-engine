/*
game.cpp and game.hpp are example files demonstrating the use of the studious
game engine. These two basic game files will generate a basic scene when the
engine is compiled and ran.
*/
#include "game.hpp"
#include "gameInstance.hpp"
#include "modelImport.hpp"
#include "textLoader.hpp"
// Global Variables, should eventually be moved to a config file
const char* soundList[] = {
    "sfx/music/endlessNight.wav"
}; // A list of gameSounds to load
const char* fragShaders[] = {
    "shaders/standardFragment.frag",
    "shaders/coll.frag",
    "shaders/text.fragmentshader"
}; // Contains collider renderer and basic object renderer.
const char* vertShaders[] = {
    "shaders/standardVertex.vert",
    "shaders/coll.vert",
    "shaders/text.vertexshader"
}; // Contains collider renderer and basic object renderer.
const char *texturePathStage[] = {
    "images/viking_room.png"
};
const char *texturePath[] = {
    "images/Sans Tex.png",
    "images/denimtexture.jpg",
    "images/shoetexture.jpg",
    "images/shirttexture.jpg"
};

int setup(pthread_mutex_t *infoLock, GameInstance *currentGame, ConfigData* config);
int runtime(pthread_mutex_t *infoLock, GameInstance *gamein);
int mainLoop(bool *updated, GameInstance *currentGame);

int main() {
    int errorNum;
    GameInstance currentGame;
    pthread_mutex_t infoLock;
    ConfigData config;
    // Run setup, if fail exit gracefully
    if (setup(&infoLock, &currentGame, &config)){
        exit(1);
    }
    //errorNum = launch(&infoLock, &currentGame);
    errorNum = runtime(&infoLock, &currentGame);
    pthread_mutex_destroy(&infoLock);
    return errorNum;
}

int setup(pthread_mutex_t *infoLock, GameInstance *currentGame, ConfigData* config){
    if (pthread_mutex_init(infoLock, NULL)) {
        printf("Mutex lock failed to engage!\n");
    }
    int flag = loadConfig(config, "misc/config.txt");
    gameInstanceArgs args;
    args.soundList = soundList;
    args.numberOfSounds = 1;
    args.vertexShaders = vertShaders;
    args.fragmentShaders = fragShaders;
    args.shaderCount = 3;
    args.lock = infoLock;
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

int runtime(pthread_mutex_t *infoLock, GameInstance *gamein){
    // Call after setup
    printf("Starting the game instance\n");

    GameInstance currentGame = *gamein;
    SDL_SetRelativeMouseMode(SDL_TRUE);

    /*
        We need proper scene loading
    */

    // Create our variables
    struct gameInfo currentGameInfo;
    bool isDone = false;
    bool updated = false;
    GLfloat stageRot[3] = {0,0,0};
    GLfloat *stageRotPointers[3] = {&stageRot[0],&stageRot[1], &stageRot[2]};

    printf("Creating camera\n");
    int gameObject[3];
    GLfloat cameraOffset[3] = {5.140022f, 1.349999f, 2.309998f};
    // Configure a new createCameraInfo struct to pass to createCamera
    cameraInfo camInfo;
    camInfo.objTarget = NULL;
    camInfo.offset = cameraOffset;
    camInfo.viewCameraAngle = 3.14159/5.0f;
    camInfo.viewAspectRatio = 16.0f / 9.0f; // 16:9 Ratio
    camInfo.viewNearClipping = 4.0f;
    camInfo.viewFarClipping = 90.0f;
    gameObject[2] = currentGame.createCamera(camInfo);

    GLint texturePattern[] = {0, 1, 2, 3};
    GLint texturePatternStage[] = {0};

    printf("Created Camera\n");

    printf("Creating map");
    //Create an importObj struct for importing the stage
    importObjInfo mapInfo;
    mapInfo.modelPath = "models/testMap1.obj";
    mapInfo.texturePath = texturePathStage;
    mapInfo.numTextures = 0;
    mapInfo.texturePattern = texturePatternStage;
    mapInfo.programID = currentGame.getProgramID(0);
    //Create a gameObjectInfo struct for creating a game object for the map
    gameObjectInfo map;
    map.characterModel = importObj(mapInfo);
    map.scaleVec = vec3(0.009500f, 0.009500f, 0.009500f);
    map.pos = vec3(-0.006f, -0.019f, 0.0f);
    map.rotateAxis = vec3(0.0f, 0.0f, 1.0f);
    map.rotAngle = 0.0f;
    map.camera = gameObject[2];
    map.collisionTagName = "map";
    map.programIDNo = currentGame.getProgramID(0);
    map.orthographic = false;
    map.colliderObject = NULL;
    gameObject[0] = currentGame.createGameObject(map);

    printf("Creating player\n");
	// The collider on the object is just a basic wire frame at the moment
	// Configure the wireframe box around the player
	importObjInfo humColInfo;
	humColInfo.modelPath = "models/rockStone.obj";
	humColInfo.texturePath = NULL;
	humColInfo.numTextures = 0;
	humColInfo.texturePattern = NULL;
	humColInfo.programID = currentGame.getProgramID(1);

	polygon *humanCollider = importObj(humColInfo);

	// Import the player object
	importObjInfo player;
	//player.modelPath = "models/tank.obj";
    player.modelPath = "models/sphere.obj";
	player.texturePath = texturePath;
	player.numTextures = 0;
	player.texturePattern = texturePattern;
	player.programID = currentGame.getProgramID(0);

	// Ready the gameObjectInfo for the player object
	gameObjectInfo playerObj;
	playerObj.characterModel = importObj(player);
	playerObj.scaleVec = vec3(0.005f, 0.005f, 0.005f); // Arbitrary hell
	playerObj.pos = vec3(0.0f, 0.0f, -1.0f);
	playerObj.rotateAxis = vec3(0.0f, 0.0f, 1.0f);
	playerObj.rotAngle = 0.0f;
	playerObj.camera = gameObject[2];
	playerObj.collisionTagName = "player";
	playerObj.programIDNo = currentGame.getProgramID(0);
	playerObj.orthographic = false;
	playerObj.colliderObject = 0;//humanCollider;

	gameObject[1] = currentGame.createGameObject(playerObj);

	printf("Creating wolf\n");
	// Import the wold object
	importObjInfo wolf;
	wolf.modelPath = "models/wolf.obj";
	wolf.texturePath = texturePath;
	wolf.numTextures = 0;
	wolf.texturePattern = texturePattern;
	wolf.programID = currentGame.getProgramID(0);
	// Ready the gameObjectInfo for the wolf object
	gameObjectInfo wolfObj;
	wolfObj.characterModel = importObj(wolf);
	wolfObj.scaleVec = vec3(0.02f, 0.02f, 0.02f); // Arbitrary hell
	wolfObj.pos = vec3(0.00f, 0.01f, -0.08f);
	wolfObj.rotateAxis = vec3(0.0f, 0.0f, 1.0f);
	wolfObj.rotAngle = 0.0f;
	wolfObj.camera = gameObject[2];
	wolfObj.collisionTagName = "NPC";
	wolfObj.programIDNo = currentGame.getProgramID(0);
	wolfObj.orthographic = false;
	wolfObj.colliderObject = humanCollider;

	gameObject[3] = currentGame.createGameObject(wolfObj);

	printf("Reserving space for text\n");
	//Create Reusable text object
	importObjInfo text;
	text.numTextures = 0;

	gameObjectInfo textObj;
	wolfObj.scaleVec = vec3(0.02f, 0.02f, 0.02f); // Arbitrary hell
	wolfObj.pos = vec3(0.00f, 0.01f, -0.08f);
	wolfObj.rotateAxis = vec3(0.0f, 0.0f, 1.0f);
	wolfObj.rotAngle = 0.0f;
	wolfObj.camera = gameObject[2];
	wolfObj.programIDNo = currentGame.getProgramID(2);



	GameCamera *currentCamera = currentGame.getCamera(gameObject[2]);
	currentCamera->setTarget(currentGame.getGameObject(gameObject[1]));
	GameObject *currentGameObject = currentGame.getGameObject(gameObject[1]);
	currentGameObject->rotateObject(stageRotPointers);
	currentGameObject = currentGame.getGameObject(gameObject[3]);
	GLfloat wolfScale = 0.01f;
	//currentGameObject -> sendScale(&wolfScale);
	// currentGameObject->rotateObject(stageRotPointers);
	currentGameObject = currentGame.getGameObject(gameObject[1]);
	printf("currentGameObject tag is %s\n", currentGameObject->getCollider());




	GLfloat xPos = -0.005f, yPos = 0.01f, zPos = 0.0f;
	GLfloat xRot = 0.0f, yRot = 180.0f, zRot = 0.0f;
	GLfloat scale = 0.0062f; // Starting scale
	GLfloat *currentPos[] = {&xPos, &yPos, &zPos};
	currentGameObject->sendPosition(currentPos);
	GLfloat *rotation[3] = {&xRot, &yRot, &zRot};
	currentGameObject->rotateObject(rotation);
	currentGameObject->sendScale(&scale);

	currentGameInfo.isDone = &isDone;
	currentGameInfo.angleX = rotation[0];
	currentGameInfo.angleY = rotation[1];
	currentGameInfo.angleZ = rotation[2];
	currentGameInfo.yPos = &yPos;
	currentGameInfo.xPos = &xPos;
	currentGameInfo.zPos = &zPos;
	currentGameInfo.scale = &scale;
	currentGameInfo.gameCamera = currentCamera;
	currentGameInfo.updated = &updated;
	currentGameInfo.currentGame = *gamein;
	currentGameInfo.infoLock = infoLock;

    /*
        End Scene Loading
    */

    // Additional threads should be added, pipes will most likely be required
    // Might also be a good idea to keep the parent thread local to watch for unexpected failures and messages from children
    pthread_t tid; // Should be moved to SDL threads for better cross platform support
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_create(&tid, &attr, rotateShape, &currentGameInfo); // spawns thread to handle input handling and object manipulation
    mainLoop(&updated, &currentGame);
    isDone = true;
    pthread_join(tid, NULL);

    return 0;
}

// Go through gameObjectLL and call drawShape method on each.
int mainLoop(bool *updated, GameInstance* gamein) {
	clock_t begin, end;
        int running = 1;
        GameInstance currentGame = *gamein;
        GLdouble deltaTime;
        short error = 0;
	while (running) {
        running = currentGame.isWindowClosed();
		begin = clock();
		currentGame.updateOGL();
        error = currentGame.updateCameras();
        error |= currentGame.updateObjects();
        error |= currentGame.updateWindow();
        if(error){
            return 1;
        }
		end = clock();
		deltaTime = (double)(end - begin) / (double)CLOCKS_PER_SEC;
        currentGame.setDeltaTime(deltaTime);
		*updated = true;
	}
	printf("Running cleanup\n");
	currentGame.cleanup();
        return 0;
}
