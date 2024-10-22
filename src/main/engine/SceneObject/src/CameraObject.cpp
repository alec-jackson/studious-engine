/**
 * @file CameraObject.cpp
 * @author Christian Galvez
 * @brief Implementation for CameraObject
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <CameraObject.hpp>
#include <algorithm>

CameraObject::CameraObject(GameObject *target, vec3 offset, float cameraAngle, float aspectRatio,
    float nearClipping, float farClipping, ObjectType type, string objectName, GfxController *gfxController) :
    SceneObject(type, objectName, gfxController), target_ { target }, offset_ { offset }, cameraAngle_ { cameraAngle },
    aspectRatio_ { aspectRatio }, nearClipping_ { nearClipping }, farClipping_ { farClipping } {}

/// @todo: Figure out what the destructor should do
CameraObject::~CameraObject() {
    if (cleanTarget_) delete target_;
}

/// @todo: Change NULL checks to nullptr
void CameraObject::render() {
    /// @todo Add field to modify target offset
    mat4 viewMatrix = lookAt(target_->getPosition(offset_), target_->getPosition(vec3(0.0f, 0.01f, 0.0f)),
        vec3(0, 1, 0));
    mat4 orthographicMatrix = ortho(0.0f, 800.0f, 0.0f, 600.0f, nearClipping_, farClipping_);
    mat4 projectionMatrix = perspective(radians(cameraAngle_), aspectRatio_,
        nearClipping_, farClipping_);
    vpMatrixPerspective_ = projectionMatrix * viewMatrix;
    vpMatrixOrthographic_ = orthographicMatrix * viewMatrix;
}

void CameraObject::update() {
    // Support target as being nullptr
    if (target_ == nullptr) {
        cleanTarget_ = true;
        target_ = new GameObject(gfxController_);
    }
    render();
    for (auto it = sceneObjects_.begin(); it != sceneObjects_.end(); ++it) {
        // Check if the object is ORTHO or PERSPECTIVE
        switch ((*it)->type()) {
            case GAME_OBJECT:
            case UI_OBJECT:
                (*it)->setVpMatrix(vpMatrixPerspective_);
                break;
            case TEXT_OBJECT:
                (*it)->setVpMatrix(vpMatrixOrthographic_);
                break;
            default:
                printf("CameraObject::update: Ignoring object [%s] type [%d]\n", (*it)->getObjectName().c_str(),
                    (*it)->type());
                break;
        }
        // Send the VP matrix for the camera to its gameobjects
        // Render the game objects
        (*it)->update();
    }
}

void CameraObject::addSceneObject(SceneObject *sceneObject) {
    sceneObjects_.push_back(sceneObject);
}

void CameraObject::removeSceneObject(SceneObject *gameObject) {
    sceneObjects_.erase(remove(sceneObjects_.begin(), sceneObjects_.end(), gameObject), sceneObjects_.end());
}
