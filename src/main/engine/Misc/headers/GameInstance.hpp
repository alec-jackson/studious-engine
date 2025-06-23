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
#include <SDL_gamecontroller.h>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <map>
#include <atomic>
#include <queue>
#include <condition_variable> //NOLINT
#include <common.hpp>
#include <ModelImport.hpp>
#include <GameObject.hpp>
#include <CameraObject.hpp>
#include <TextObject.hpp>
#include <SpriteObject.hpp>
#include <UiObject.hpp>
#include <TileObject.hpp>
#include <config.hpp>
#include <AnimationController.hpp>

// Number of samples to use for anti-aliasing
#define AASAMPLES 8
#define SE_NO_INPUT 0xBEEF

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
    vector<std::shared_ptr<CameraObject>> cameras_;
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
    mutex progressLock_;
    std::condition_variable inputCv_;
    std::condition_variable progressCv_;
    queue<std::function<void(void)>> protectedGfxReqs_;
    queue<int> inputQueue_;
    bool audioInitialized_ = false;

    void initWindow();
    void initAudio();
    void initController();
    void initApplication();
    /**
     * @brief Runs GFX requests on the main thread. This method is required when calling GfxController methods
     * from separate threads.
     */
    void runGfxRequests();
    /**
     * @brief Updates game objects in the game scene.
     */
    int updateObjects();
    /**
     * @brief Updates the underlying SDL window.
     */
    int updateWindow();
    /**
     * @brief Polls for new input events and pushes them to the input queue.
     */
    void updateInput();

 public:
    GameInstance(GfxController *gfxController, AnimationController *animationController, int width,
        int height);
    ~GameInstance();
    void init();
    GameObject *createGameObject(Polygon *characterModel, vec3 position, vec3 rotation, float scale,
        string objectName);
    CameraObject *createCamera(SceneObject *target, vec3 offset, float cameraAngle, float aspectRatio,
              float nearClipping, float farClipping);
    TextObject *createText(string message, vec3 position, float scale, string fontPath, float charSpacing,
        int charPoint, string objectName);
    SpriteObject *createSprite(string spritePath, vec3 position, float scale,
        ObjectAnchor anchor, string objectName);
    UiObject *createUi(string spritePath, vec3 position, float scale, float wScale, float hScale,
        ObjectAnchor anchor, string objectName);
    TileObject *createTileMap(map<string, string> textures, vector<TileData> mapData,
        vec3 position, float scale, ObjectAnchor anchor, string objectName);
    int getWidth();
    int getHeight();
    vec3 getResolution();
    vec3 getDirectionalLight();
    /**
     * @brief Performs a "protected" GFX request. This adds a GfxRequest to the internal gfx request queue
     * to be run on the main thread. This function blocks until the request is fulfilled.
     * @return True when the request is fulfilled, otherwise return false.
     */
    bool protectedGfxRequest(std::function<void(void)> req);
    const Uint8 *getKeystate();
    const bool getControllerInput(SDL_GameControllerButton button);
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
    /**
     * @brief Updates controllers and game objects in the current game. Controllers updated with this function
     * are controllers that must be updated from the main thread.
     */
    int update();
    /**
     * @brief Fetches input from the internal input queue. Functions blocks until an input event is received.
     */
    int getInput(bool blocking);
    int lockScene();
    int unlockScene();
    /**
     * @brief Shuts down the current game. This updates the internal shutdown flag and causes all blocking methods
     * to immediately return. Separate game threads should key off of the shutdown flag to shutdown gracefully.
     * @see GameInstance::isShutDown().
     */
    void shutdown();
    /**
     * @brief Blocks until a specific key is pressed.
     * @param input The input to wait for.
     * @return True when the key is received, false is shutdown signal received.
     */
    bool waitForKeyDown(int input);

    /**
     * @brief Blocks until a specific key from the passed in list is pressed.
     * @param input List of inputs for wait for.
     * @return True when the key is received, false is shutdown signal received.
     */
    bool waitForKeyDown(vector<int> input);
    /**
     * @brief Checks if the game has been shut down.
     * @return Returns true if shutdown has been called, false otherwise.
     */
    inline int isShutDown() { return shutdown_; }
    /**
     * @brief Blocks caller until the provided predicate is satisfied. Must be signaled manually when
     * values in the predicate are altered. @see GameInstance::signalProgress().
     */
    bool waitForProgress(std::function<bool(void)> pred);
    /**
     * @brief Signals to the game instance that it should wakeup all threads blocked by
     * GameInstance::waitForProgress so they can check if their predicate has been satisfied.
     */
    inline void signalProgress() { progressCv_.notify_all(); }
    /**
     * @brief Enables/disables use of Vsync.
     */
    void configureVsync(bool enable);
};
