#ifndef GAMEINSTANCE_H
#define GAMEINSTANCE_H
#include "common.hpp"
#include "modelImport.hpp"
#include "gameObject.hpp"
#include "shaderLoader.hpp"
#include "textLoader.hpp"

/*
 gameObjectLL is a struct used for holding a Linked List of all of the active
 GameObjects in the scene. The next pointer points to another dynamically
 allocated gameObjectLL struct, and current contains the GameObject contained
 within the current gameObjectLL entry.
*/
typedef struct gameObjectLL {
	struct gameObjectLL *next;
	struct GameObject *current;
} gameObjectLL;

/*
 gameCameraLL works the same as gameObjectLL, however this Linked List
 exclusively contains all of the active cameras in the current game scene.
*/
typedef struct gameCameraLL {
	struct gameCameraLL *next;
	struct GameCamera *current;
} gameCameraLL;

/*
 controllerReadout is used for getting input from a controller. This struct
 will be used in conjunction with SDL_GameControllerGetAxis to get input from
 the left controller stick.
*/
typedef struct controllerReadout {
	Sint16 leftAxis;
} controllerReadout;

/*
 gameInstanceArgs contains all of the arguments needed for the startGameInstance
 method inside the GameInstance class. The following struct members are used
 in the following manner:
 * int windowWidth - Defines the width of the window in pixels
 * int windowHeight - Defines the height of the window in pixels
 * vector<string> soundList - Contains paths to sound effects to be used in the
 	current GameInstance.
 * vector<string> vertexShaders - Contains paths to vertex shaders to be used in
 	the current GameInstance.
 * vector<string> fragmentShaders - Contains paths to fragment shaders to be
 	used in the current GameInstance.
*/
typedef struct gameInstanceArgs {
	int windowWidth;
	int windowHeight;
	vector<string> soundList;
	vector<string> vertexShaders;
	vector<string> fragmentShaders;
} gameInstanceArgs;

/*
 The GameInstance class is the class that holds all of the information about the
 current game scene. Methods inside of this class are used to operate on
 most of the objects contained within the game scene. This class contains the
 Linked List containing active GameObjects, as well as a Linked List containing
 all of the active GameCameras. Objects inside of this class should be
 referenced externally through public get/set methods described in this class.
*/
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
	int width, height;
	textLib text;

	void initWindow(int width, int height);
	void initAudio();
	void initController();
	void initApplication(vector<string> vertexPath, vector<string> fragmentPath);

public:
	void startGameInstance(gameInstanceArgs args);
	int createGameObject(gameObjectInfo objectInfo);
	int createCamera(cameraInfo camInfo);
	int getWidth();
	int getHeight();
	vec3 getDirectionalLight();
	const Uint8 *getKeystate();
	GLuint getProgramID(uint index);
	controllerReadout *getControllers(int controllerIndex);
	int getControllersConnected();
	void playSound(int soundIndex, int loop);
	void changeWindowMode(int mode);
	void cleanup();
	int destroyGameObject(GameObject *object);
	GameObject *getGameObject(int gameObjectID);
	GameCamera *getCamera(int gameCameraID);
	GLdouble getDeltaTime();
	int setDeltaTime(GLdouble time);
	void setLuminance(GLfloat luminanceValue);
	void basicCollision(GameInstance* gameInstance);
	bool isWindowOpen();
	void updateOGL();
	int updateCameras();
	int updateObjects();
	int updateWindow();
};

#endif
