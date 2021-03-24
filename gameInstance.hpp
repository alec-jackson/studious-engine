#pragma once
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_mixer.h>
#include <glm/gtc/matrix_transform.hpp>


using namespace glm;

#include "modelImport.hpp"
#include "gameObject.hpp"
#include "shaderLoader.hpp"
#include "textLoader.hpp"



// Linked List of GameObjects
typedef struct gameObjectLL {
	struct gameObjectLL *next;
	struct GameObject *current;
} gameObjectLL;

// Contains all of the cameras being used in the current instance
typedef struct gameCameraLL {
	struct gameCameraLL *next;
	struct GameCamera *current;
} gameCameraLL;

typedef struct controllerReadout {
	Sint16 leftAxis;
} controllerReadout;

class GameInstance {
private:
	const Uint8 *keystate;
	const char **sfxNames;

	int sfxCount, gameObjectCount, audioID, controllersConnected, gameCameraCount, numShaders;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Surface* screenSurface;
	SDL_Event event;
	Mix_Chunk **sound;
	gameObjectLL *gameObjects;
	gameCameraLL *gameCameras;
	GLuint *programID, vertexArrayID;
	GLdouble deltaTime;
	SDL_GameController *gameControllers[2];
	controllerReadout controllerInfo[2];
	vec3 directionalLight;
	GLfloat luminance;
	pthread_mutex_t *infoLock;
	int width, height;
	textLib text;

	void initWindow(int width, int height);
	void initAudio();
	void initController();
	void initApplication(const char** vertexPath, const char** fragmentPath);

public:
	//GameInstance(); //Constructor

	void startGameInstance(int windowWidth, int windowHeight, const char *soundList[], int numberOfSounds,
	          const char** vertexShaders, const char** fragmentShaders, int shaderCount, pthread_mutex_t *lock);

	int createGameObject(gameObjectInfo objectInfo);

	int createCamera(cameraInfo camInfo);

	//SDL_Window *getWindow();
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
	void performOGL();
	int updateCameras();
	int updateObjects();
	int updateWindow();
};
