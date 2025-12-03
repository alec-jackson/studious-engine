/**
 * @file GameScene.hpp
 * @brief Class definition for GameScene.
 * @author Christian Galvez
 * @date 09-13-25
 * @copyright Studious Engine 2025
 */
#pragma once

#include <memory.h>
#include <map>
#include <string>
#include <memory>
#include <vector>
#include <SceneObject.hpp>
#include <CameraObject.hpp>

class GameScene {
 public:
    explicit inline GameScene(std::string sceneName) : sceneName_ { sceneName } {}

    void addSceneObject(std::shared_ptr<SceneObject> sceneObject);
    void removeSceneObject(std::string objectName);
    std::shared_ptr<SceneObject> getSceneObject(std::string objectName);

    void update(CameraObject *camera);

    void loadGameScene(std::string path);
    void saveGameScene(std::string path);

    // MISC
    void setDirectionalLight(vec3 directionalLight);
    inline vec3 getDirectionalLight() { return directionalLight_; }
    inline string getSceneName() const { return sceneName_; }
    inline const std::map<std::string, std::shared_ptr<SceneObject>> getSceneObjects() const { return sceneObjects_; }
    void refresh();

 private:
    void resetRenderPriorityMap();
    std::string sceneName_;
    std::map<std::string, std::shared_ptr<SceneObject>> sceneObjects_;
    // Render priority to list of scene objects
    std::map<uint, std::vector<std::shared_ptr<SceneObject>>> renderPriorityMap_;
    vec3 directionalLight_ = vec3(-100, 100, 100);
    std::mutex sceneLock_;
};
