/**
 * @file gameInstance.hpp
 * @author Christian Galvez, Alec Jackson
 * @brief GameInstance class contains a current scene to render GameObjects in
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once
#include <string>
#include <vector>
#include <common.hpp>
#include <ModelImport.hpp>
#include <GameObject.hpp>
#include <CameraObject.hpp>
#include <TextObject.hpp>

// Number of samples to use for anti-aliasing
#define AASAMPLES 8

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
/// @todo This is bunk and will be refactored eventually...
typedef struct gameInstanceArgs {
    int windowWidth;
    int windowHeight;
    vector<string> soundList;
    vector<string> vertexShaders;
    vector<string> fragmentShaders;
    GfxController *gfxController;
} gameInstanceArgs;

/*
 The GameInstance class is the class that holds all of the information about the
 current game scene. Methods inside of this class are used to operate on
 most of the objects contained within the game scene. This class contains the
 Linked List containing active GameObjects, as well as a Linked List containing
 all of the active CameraObjects. Objects inside of this class should be
 referenced externally through public get/set methods described in this class.
*/
class GameInstance {
 private:
    const Uint8 *keystate;
    vector<string> sfxNames;
    int audioID, controllersConnected;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface* screenSurface;
    SDL_Event event;
    vector<Mix_Chunk *> sound;
    vector<GameObject *> gameObjects;
    vector<CameraObject *> gameCameras;
    vector<TextObject *> gameTexts;
    GLuint vertexArrayID;
    GLdouble deltaTime;
    SDL_GameController *gameControllers[2];
    controllerReadout controllerInfo[2];
    vec3 directionalLight;
    GLfloat luminance;
    int width, height;
    mutex sceneLock;
    GfxController *gfxController_;

    void initWindow(int width, int height);
    void initAudio();
    void initController();
    void initApplication(vector<string> vertexPath, vector<string> fragmentPath);

 public:
    void startGameInstance(gameInstanceArgs args);
    int createGameObject(gameObjectInfo objectInfo);
    int createCamera(cameraInfo camInfo);
    int createText(textObjectInfo info);
    int getWidth();
    int getHeight();
    vec3 getDirectionalLight();
    const Uint8 *getKeystate();
    controllerReadout *getControllers(int controllerIndex);
    int getControllersConnected();
    void playSound(int soundIndex, int loop);
    void changeWindowMode(int mode);
    void cleanup();
    int destroyGameObject(GameObject *object);
    GameObject *getGameObject(uint gameObjectID);
    CameraObject *getCamera(uint gameCameraID);
    TextObject *getText(uint gameTextID);
    GLdouble getDeltaTime();
    int getCollision(GameObject *object1, GameObject *object2, vec3 moving);
    int setDeltaTime(GLdouble time);
    void setLuminance(GLfloat luminanceValue);
    void basicCollision(GameInstance* gameInstance);
    bool isWindowOpen();
    void updateOGL();
    int updateCameras();
    int updateObjects();
    int updateWindow();
    int lockScene();
    int unlockScene();
};
