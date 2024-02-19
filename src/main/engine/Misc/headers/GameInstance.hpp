/**
 * @file GameInstance.hpp
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
    int audioID, controllersConnected;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Surface* screenSurface;
    SDL_Event event;
    vector<Mix_Chunk *> sound;
    vector<GameObject *> gameObjects;
    vector<CameraObject *> gameCameras;
    vector<TextObject *> gameTexts;
    vector<string> soundList_;
    vector<string> fragShaders_;
    vector<string> vertShaders_;
    vector<string> texturePathStage_;
    vector<string> texturePath_;
    GLdouble deltaTime;
    SDL_GameController *gameControllers[2];
    controllerReadout controllerInfo[2];
    vec3 directionalLight;
    GLfloat luminance;
    int width_, height_;
    mutex sceneLock;
    GfxController *gfxController_;

    void initWindow(int width, int height);
    void initAudio();
    void initController();
    void initApplication(vector<string> vertexPath, vector<string> fragmentPath);

 public:
    GameInstance(vector<string> soundList, vector<string> vertShaders,
        vector<string> fragShaders, GfxController *gfxController, int width, int height);
    void startGame();
    int createGameObject(Polygon *characterModel, vec3 position, vec3 rotation, GLfloat scale, int camera,
        string objectName);
    int createCamera(GameObject *target, vec3 offset, GLfloat cameraAngle, GLfloat aspectRatio,
              GLfloat nearClipping, GLfloat farClipping);
    int createText(string message, string fontPath, GLuint programId, string objectName);
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
