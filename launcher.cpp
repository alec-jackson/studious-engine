#ifndef REQUIRED_LIBS
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <math.h>
#include <cstring>
#include <vector>
#include <pthread.h>
#include <unistd.h>
#define SDL2_image
#include <GL/glew.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;
using namespace std;
#define REQUIRED_LIBS
#endif

#include "launcher.hpp"
#include "shaderLoader.hpp"
#include "modelImport.hpp"
#include "gameObject.hpp"
#include "gameInstance.hpp"
#include "inputMonitor.hpp"
#include "physics.hpp"

#define PI 3.14159265

//Analog joystick dead zone
const int JOYSTICK_DEAD_ZONE = 8000;

GameInstance currentGame;

// Handles launch options
// GameObject startGameInstance is used to actually start the instance, this is used to configure the instance
// Should probably add flag parsing at some point for further call modularity
// Function should probably also set system interupt handling
int launch (pthread_mutex_t *infoLock, GameInstance *gamein) {
	const char* soundList[] = {"sfx/music/endlessNight.wav"};
	currentGame = *gamein;
	printf("Starting the game instance\n");
	SDL_SetRelativeMouseMode(SDL_TRUE);
	/* Just for a reminder
	void configureGameObject(polygon *characterModel, vec3 scaleVec, vec3 pos, vec3 rotateAxis, GLfloat rotAngle, vec3 cameraPos, const char *collisionTagName, GLuint programID, bool orthographic)
	*/
	printf("Creating camera\n");
	int gameObject[4];
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

	const char *texturePathStage[] = {"images/viking_room.png"};
	const char *texturePath[] = {"images/Sans Tex.png", "images/denimtexture.jpg", "images/shoetexture.jpg", "images/shirttexture.jpg"};
	GLint texturePattern[] = {0, 1, 2, 3};
	GLint texturePatternStage[] = {0};
	bool updated = false;
	//GameCamera *currentCamera = currentGame.getGameCamera(gameObject[2]);
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

	GLfloat stageRot[3] = {0,0,0};
	GLfloat *stageRotPointers[3] = {&stageRot[0],&stageRot[1], &stageRot[2]};

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

	// Import the sans player object
	importObjInfo sans;
	sans.modelPath = "models/tank.obj";
	sans.texturePath = texturePath;
	sans.numTextures = 0;
	sans.texturePattern = texturePattern;
	sans.programID = currentGame.getProgramID(0);

	// Ready the gameObjectInfo for the sans object
	gameObjectInfo sansObj;
	sansObj.characterModel = importObj(sans);
	sansObj.scaleVec = vec3(0.005f, 0.005f, 0.005f); // Arbitrary hell
	sansObj.pos = vec3(0.0f, 0.0f, -1.0f);
	sansObj.rotateAxis = vec3(0.0f, 0.0f, 1.0f);
	sansObj.rotAngle = 0.0f;
	sansObj.camera = gameObject[2];
	sansObj.collisionTagName = "player";
	sansObj.programIDNo = currentGame.getProgramID(0);
	sansObj.orthographic = false;
	sansObj.colliderObject = humanCollider;

	gameObject[1] = currentGame.createGameObject(sansObj);

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
	bool isDone = false;
	gameInfo currentGameInfo;
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


	// Additional threads should be added, pipes will most likely be required
	// Might also be a good idea to keep the parent thread local to watch for unexpected failures and messages from children
	pthread_t tid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_create(&tid, &attr, rotateShape, &currentGameInfo); // spawns thread to handle input handling and object manipulation
	currentGame.mainLoop(&updated);
	isDone = true;
	pthread_join(tid, NULL);

	return 0;
}
