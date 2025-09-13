#pragma once

#include <memory.h>
#include <map>
#include <SceneObject.hpp>
#include <CameraObject.hpp>
/**
 * Note for me: A GameScene is essentially a collection of SceneObjects that define a scene. The CameraObject will be
 * independent of a Scene, but should be passed to the GameScene for rendering. For now, there should only
 * be one active camera at a time ever. The camera is managed by the GameInstance, and applies transformations
 * to objects in a GameScene.
 *
 * Should the object creation methods be moved over to the scene itself? Maybe not, since we probably don't want to
 * keep pointers for each individual scene around... Probably best to modify them in the GameInstance class to just
 * create an inject scene objects into a game scene.
 *
 * How should we handle rendering? Right now it's handled by the camera. I think we should have an update() function
 * inside of the GameScene itself, and the current camera calls it with its VP matrix passed as an arg. That should
 * make scene switching really easy.
 *
 * Will probably need to call update() on the camera first, then update on the GameScene.
 */
class GameScene {
 public:
    explicit inline GameScene(std::string sceneName) : sceneName_ { sceneName } {}

    void addSceneObject(std::shared_ptr<SceneObject> sceneObject);
    void removeSceneObject(std::string objectName);
    std::shared_ptr<SceneObject> getSceneObject(std::string objectName);

    void update(CameraObject *camera);

    void loadGameScene(std::string *path);
    void saveGameScene(std::string *path);

    // MISC
    void setDirectionalLight(vec3 directionalLight);
    inline vec3 getDirectionalLight() { return directionalLight_; }
    inline string getSceneName() const { return sceneName_; }
 private:
    void resetRenderPriorityMap();
    std::string sceneName_;
    std::map<std::string, std::shared_ptr<SceneObject>> sceneObjects_;
    // Render priority to list of scene objects
    map<uint, vector<std::shared_ptr<SceneObject>>> renderPriorityMap_;
    vec3 directionalLight_ = vec3(-100, 100, 100);
    std::mutex sceneLock_;
};
