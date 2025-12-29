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
#include <SDL_scancode.h>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <map>
#include <atomic>
#include <queue>
#include <condition_variable> //NOLINT
#include <common.hpp>
#include <ColliderExt.hpp>
#include <SceneObject.hpp>
#include <ModelImport.hpp>
#include <GameObject.hpp>
#include <CameraObject.hpp>
#include <TextObject.hpp>
#include <SpriteObject.hpp>
#include <UiObject.hpp>
#include <TileObject.hpp>
#include <config.hpp>
#include <physics.hpp>
#include <AnimationController.hpp>
#include <GameScene.hpp>
#include <InputController.hpp>
#include <TPSCameraObject.hpp>
#include <FPSCameraObject.hpp>
#include <studious_utility.hpp>

// Number of samples to use for anti-aliasing
#define AASAMPLES 8

extern double deltaTime;

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
    GfxController *gfxController_;
    AnimationController *animationController_;
    PhysicsController *physicsController_;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;
    SDL_GLContext mainContext;
    vector<std::shared_ptr<CameraObject>> cameras_;
    std::shared_ptr<CameraObject> activeCamera_;
    vector<string> vertShaders_;
    vector<string> fragShaders_;
    vector<string> texturePathStage_;
    vector<string> texturePath_;
    map<string, Mix_Chunk *> loadedSounds_;
    map<string, int> activeChannels_;
    float luminance;
    int width_, height_;
    int vsync_;
    int audioID;
    std::atomic<int> shutdown_;
    mutex sceneLock_;
    mutex soundLock_;
    mutex requestLock_;
    mutex inputLock_;
    mutex progressLock_;
    std::condition_variable inputCv_;
    std::condition_variable progressCv_;
    queue<std::function<void(void)>> protectedGfxReqs_;
    queue<GameInput> inputQueue_;
    bool audioInitialized_ = false;
    std::shared_ptr<GameScene> activeScene_;
    map<string, std::shared_ptr<GameScene>> gameScenes_;

    void initWindow();
    void initAudio();
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

    bool addSceneObject(std::shared_ptr<SceneObject> sceneObject);
    std::shared_ptr<GameScene> getGameScene_(string sceneName);

 public:
    explicit GameInstance(const StudiousConfig &config);
    ~GameInstance();
    void init();
    GameObject *createGameObject(std::shared_ptr<Polygon> characterModel, vec3 position, vec3 rotation, float scale,
        string objectName);
    CameraObject *createCamera(SceneObject *target, vec3 offset, float cameraAngle, float aspectRatio,
              float nearClipping, float farClipping, string cameraName);
    TPSCameraObject *createTPSCamera(SceneObject *target, vec3 offset, float cameraAngle, float aspectRatio,
              float nearClipping, float farClipping, string cameraName);
    FPSCameraObject *createFPSCamera(SceneObject *target, vec3 offset, vec3 camPos, float cameraAngle,
        float aspectRatio, float nearClipping, float farClipping, string cameraName);
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
    int playSound(string sfxName, bool loop, int volume);
    int loadSound(string sfxName, string sfxPath);
    int changeVolume(string sfxName, int volume);
    int stopSound(string sfxName);
    void changeWindowMode(int mode);
    int destroySceneObject(SceneObject *object);
    SceneObject *getSceneObject(string objectName);
    CameraObject *getCamera(string cameraName);

    template<typename T>
    inline T *getSceneObject(string objectName) { return dynamic_cast<T*>(getSceneObject(objectName)); }
    template<typename T>
    inline T *getCamera(string cameraName) { return dynamic_cast<T*>(getCamera(cameraName)); }
    int removeSceneObject(string objectName);
    /* NOTE - getCollision functions are for convenience and will be deprecated with the physics controller */
    int getCollision(SceneObject *object1, SceneObject *object2);
    void setLuminance(float luminanceValue);
    void setDirectionalLight(vec3 directionalLight);
    void basicCollision(GameInstance* gameInstance);
    /**
     * @brief Updates controllers and game objects in the current game. Controllers updated with this function
     * are controllers that must be updated from the main thread.
     */
    int update();
    /**
     * @brief Fetches input from the internal input queue. Functions blocks until an input event is received.
     * @return GameInput value pressed from either a controller or keyboard.
     */
    GameInput getInput();
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
    bool waitForInput(GameInput input);

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
    void processConfig(const StudiousConfig &config);
    void createGameScene(string sceneName);
    void loadGameSceneFromFile(string path);
    inline std::shared_ptr<GameScene> getActiveScene() { return activeScene_; }
    void setActiveScene(string sceneName);
    std::shared_ptr<GameScene> getGameScene(string sceneName);
    void setActiveCamera(string cameraName);
    std::shared_ptr<CameraObject> getActiveCamera();
    template<typename T>
    inline SHD(T) getActiveCamera() { return std::dynamic_pointer_cast<T>(getActiveCamera()); }
};
