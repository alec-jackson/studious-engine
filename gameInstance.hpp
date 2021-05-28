#pragma once
#include "common.hpp"
#include "modelImport.hpp"
#include "gameObject.hpp"
#include "shaderLoader.hpp"
#include "textLoader.hpp"

// Linked List of active GameObjects in Scene
typedef struct gameObjectLL {
	struct gameObjectLL *next;
	struct GameObject *current;
} gameObjectLL;

// Linked List of active Cameras in Scene
typedef struct gameCameraLL {
	struct gameCameraLL *next;
	struct GameCamera *current;
} gameCameraLL;

// Used for reading controller input
typedef struct controllerReadout {
	Sint16 leftAxis;
} controllerReadout;

// Arguments for startGameInstance method
typedef struct gameInstanceArgs {
	int windowWidth;
	int windowHeight;
	vector<string> soundList;
	vector<string> vertexShaders;
	vector<string> fragmentShaders;
	mutex *lock;
} gameInstanceArgs;

class GameInstance {
private:
	const Uint8 *keystate;
	vector<string> sfxNames;
	int gameObjectCount, audioID, controllersConnected, gameCameraCount;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Surface* screenSurface;
	SDL_Event event;
	vector<Mix_Chunk *> sound ;
	gameObjectLL *gameObjects;
	gameCameraLL *gameCameras;
	vector<GLuint> programID;
	GLuint vertexArrayID;
	GLdouble deltaTime;
	SDL_GameController *gameControllers[2];
	controllerReadout controllerInfo[2];
	vec3 directionalLight;
	GLfloat luminance;
	mutex *infoLock;
	int width, height;
	textLib text;

	void initWindow(int width, int height);
	void initAudio();
	void initController();
	void initApplication(vector<string> vertexPath, vector<string> fragmentPath);

public:
	//GameInstance(); //Constructor
	void startGameInstance(gameInstanceArgs args);
	int createGameObject(gameObjectInfo objectInfo);
	int createCamera(cameraInfo camInfo);
	int getWidth();
	int getHeight();
	vec3 getDirectionalLight();
	const Uint8 *getKeystate();
	GLuint getProgramID(int index);
	controllerReadout *getControllers(int controllerIndex);
	int getControllersConnected();
	void swapBuffers();
	void playSound(int soundIndex, int loop);
	void changeWindowMode(int mode);
	void cleanup();
	int destroyGameObject(GameObject *object);
	void mainLoop(bool *updated);
	GameObject *getGameObject(int gameObjectID);
	GameCamera *getCamera(int gameCameraID);
	GLdouble *getDeltaTime();
	int setDeltaTime(GLdouble time);
	void setLuminance(GLfloat luminanceValue);
	void basicCollision(GameInstance* gameInstance);
	int isWindowClosed();
	void updateOGL();
	int updateCameras();
	int updateObjects();
	int updateWindow();
};
