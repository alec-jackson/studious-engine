/**
 * @file GameScene.cpp
 * @brief Implementation for GameScene class. The GameScene contains a collection of SceneObjects
 * that define a scene.
 * @author Christian Galvez
 * @date 09-13-2025
 * @copyright Studious Engine 2025
 */
#include <GameScene.hpp>
#include <cassert>
#include <SceneObject.hpp>
#include <mutex>

void GameScene::addSceneObject(std::shared_ptr<SceneObject> sceneObject) {
    std::unique_lock<std::mutex> scopeLock(sceneLock_);
    if (!sceneObject.get()) {
        fprintf(stderr,
            "GameScene::addSceneObject: Cannot add invalid scene object!\n");
        return;
    }
    assert(!sceneObject->objectName().empty());
    // SANITY CHECK - ENFORCE OBJECTS CANNOT HAVE SAME NAME!!!!!!!!
    auto soit = sceneObjects_.find(sceneObject->objectName());
    if (soit != sceneObjects_.end()) {
        fprintf(stderr,
            "GameScene::addSceneObject: ERROR! Cannot add two objects with the same name! [%s]\n",
            sceneObject->objectName().c_str());
        assert(0);
    }
    sceneObjects_[sceneObject->objectName()] = sceneObject;
    renderPriorityMap_[sceneObject->getRenderPriority()].push_back(sceneObject);
}

void GameScene::removeSceneObject(std::string objectName) {
    std::unique_lock<std::mutex> scopeLock(sceneLock_);
    auto soit = sceneObjects_.find(objectName);
    if (soit != sceneObjects_.end()) {
        sceneObjects_.erase(soit);
        resetRenderPriorityMap();
    } else {
        fprintf(stderr,
            "GameScene::removeSceneObject: %s not found\n",
            objectName.c_str());
    }
}

void GameScene::refresh() {
    std::unique_lock<std::mutex> scopeLock(sceneLock_);
    // Rebuild the render map
    renderPriorityMap_.clear();
    for (auto obj : sceneObjects_) {
        renderPriorityMap_[obj.second->getRenderPriority()].push_back(obj.second);
    }
}

void GameScene::update(CameraObject *camera) {
    std::unique_lock<std::mutex> scopeLock(sceneLock_);
    if (camera == nullptr) {
        fprintf(stderr, "GameScene::update: Camera missing!\n");
        return;
    }
    auto resolution = camera->getResolution();
    auto perspectiveMat = camera->getPerspective();
    auto orthoMat = camera->getOrthographic();
    auto orthoMatBase = camera->getOrthographicBase();
    for (auto &obj : renderPriorityMap_) {
        printf("GameScene::update: Rendering level %d\n", obj.first);
        // Send the current screen res to each object
        /// @todo Maybe use a global variable for resolution?
        auto objList = obj.second;
        for (auto objPtr : objList) {
            printf("GameScene::update: Rendering object %s\n", objPtr->objectName().c_str());
            objPtr->setResolution(resolution);
            // Check if the object is ORTHO or PERSPECTIVE
            switch (objPtr->type()) {
                case GAME_OBJECT:
                    objPtr->setVpMatrix(perspectiveMat);
                    break;
                case UI_OBJECT:
                    /* Do not apply view to UI */
                    objPtr->setVpMatrix(orthoMatBase);
                    break;
                case SPRITE_OBJECT:
                case TEXT_OBJECT:
                case TILE_OBJECT:
                    objPtr->setVpMatrix(orthoMat);
                    break;
                default:
                    printf("CameraObject::update: Ignoring object [%s] type [%d]\n", objPtr->objectName().c_str(),
                        objPtr->type());
                    break;
            }
            // Render the object -> the map iterator will sort keys automatically
            objPtr->update();
        }
    }
}

void GameScene::resetRenderPriorityMap() {
    renderPriorityMap_.clear();
    for (auto obj : sceneObjects_) {
        renderPriorityMap_[obj.second.get()->getRenderPriority()].push_back(obj.second);
    }
}

std::shared_ptr<SceneObject> GameScene::getSceneObject(std::string objectName) {
    std::unique_lock<std::mutex> scopeLock(sceneLock_);
    std::shared_ptr<SceneObject> result;
    auto soit = sceneObjects_.find(objectName);
    if (soit != sceneObjects_.end()) {
        result = soit->second;
    }
    return result;
}

void GameScene::setDirectionalLight(vec3 directionalLight) {
    std::unique_lock<std::mutex> scopeLock(sceneLock_);
    directionalLight_ = directionalLight;
    // Send new directional light to 3D gameobjects
    for (auto obj : sceneObjects_) {
        // Make this a light ext eventually...
        if (obj.second.get()->type() == GAME_OBJECT) {
            GameObject *cObj = static_cast<GameObject *>(obj.second.get());
            cObj->setDirectionalLight(directionalLight);
        }
    }
}

void loadGameScene(std::string path) {
    // Use YAML parser here ...
    printf("Not implemented yet! %s\n", path.c_str());
}

void saveGameScene(std::string path) {
    // Use YAML writer here...
    printf("Not implemented yet! %s\n", path.c_str());
}
