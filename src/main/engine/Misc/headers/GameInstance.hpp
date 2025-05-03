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
#include <algorithm>
#include <memory>
#include <map>
#include <atomic>
#include <queue>
#include <common.hpp>
#include <ModelImport.hpp>
#include <GameObject.hpp>
#include <CameraObject.hpp>
#include <TextObject.hpp>
#include <SpriteObject.hpp>
#include <UiObject.hpp>
#include <config.hpp>
#include <AnimationController.hpp>

// Number of samples to use for anti-aliasing
#define AASAMPLES 8

extern double deltaTime;

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
    GfxController *gfxController_;
    AnimationController *animationController_;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;
    SDL_GLContext mainContext;
    vector<std::shared_ptr<SceneObject>> sceneObjects_;
    vector<string> vertShaders_;
    vector<string> fragShaders_;
    vector<string> texturePathStage_;
    vector<string> texturePath_;
    map<string, Mix_Chunk *> loadedSounds_;
    map<string, int> activeChannels_;
    SDL_GameController *gameControllers[2];
    controllerReadout controllerInfo[2];
    vec3 directionalLight;
    float luminance;
    int width_, height_;
    int audioID, controllersConnected = 0;
    std::atomic<int> shutdown_;
    mutex sceneLock_;
    mutex soundLock_;
    mutex requestLock_;
    mutex inputLock_;
    std::condition_variable inputCv_;
    queue<std::function<void(void)>> protectedGfxReqs_;
    queue<SDL_Scancode> inputQueue_;
    bool audioInitialized_ = false;

    void initWindow(const configData &config);
    void initAudio();
    void initController();
    void initApplication(vector<string> vertexPath, vector<string> fragmentPath);
    void runGfxRequests();

 public:
    GameInstance(vector<string> vertShaders,
        vector<string> fragShaders, GfxController *gfxController, AnimationController *animationController, int width, int height);
    ~GameInstance();
    void startGame(const configData &config);
    GameObject *createGameObject(Polygon *characterModel, vec3 position, vec3 rotation, float scale,
        string objectName);
    CameraObject *createCamera(SceneObject *target, vec3 offset, float cameraAngle, float aspectRatio,
              float nearClipping, float farClipping);
    TextObject *createText(string message, vec3 position, float scale, string fontPath, float charSpacing,
        int charPoint, uint programId, string objectName);
    SpriteObject *createSprite(string spritePath, vec3 position, float scale, unsigned int programId,
        ObjectAnchor anchor, string objectName);
    UiObject *createUi(string spritePath, vec3 position, float scale, float wScale, float hScale,
        unsigned int programId, ObjectAnchor anchor, string objectName);
    int getWidth();
    int getHeight();
    vec3 getResolution();
    vec3 getDirectionalLight();
    void protectedGfxRequest(std::function<void(void)> req);
    const Uint8 *getKeystate();
    controllerReadout *getControllers(int controllerIndex);
    int getControllersConnected();
    int playSound(string sfxName, bool loop, int volume);
    int loadSound(string sfxName, string sfxPath);
    int changeVolume(string sfxName, int volume);
    int stopSound(string sfxName);
    void changeWindowMode(int mode);
    int destroySceneObject(SceneObject *object);
    SceneObject *getSceneObject(string objectName);
    int removeSceneObject(string objectName);
    /* NOTE - getCollision functions are for convenience and will be deprecated with the physics controller */
    int getCollision(GameObject *object1, GameObject *object2, vec3 moving);
    int getCollision2D(GameObject2D *object1, GameObject2D *object2, vec3 moving);
    void setLuminance(float luminanceValue);
    void basicCollision(GameInstance* gameInstance);
    bool isWindowOpen();
    int updateObjects();
    int updateWindow();
    void updateInput();
    SDL_Scancode getInput();
    int lockScene();
    int unlockScene();
    void shutdown();
    bool waitForKeyDown(SDL_Scancode input);
    inline int isShutDown() { return shutdown_; }
};
