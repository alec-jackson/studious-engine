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
#include <string>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <CameraObject.hpp>

CameraObject::CameraObject(SceneObject *target, vec3 offset, float cameraAngle, float aspectRatio,
    float nearClipping, float farClipping, ObjectType type, string objectName, GfxController *gfxController) :
    SceneObject(type, objectName, gfxController), target_ { target }, offset_ { offset }, cameraAngle_ { cameraAngle },
    aspectRatio_ { aspectRatio }, nearClipping_ { nearClipping }, farClipping_ { farClipping } {
        initialTargetPos_ = target != nullptr ? target->getPosition() : vec3(0);
    }

/// @todo: Figure out what the destructor should do
CameraObject::~CameraObject() {
}

void CameraObject::render() {
    vec3 eye = vec3(0);
    vec3 center = vec3(0.0f, 0.01f, 0.0f);
    orthographicMatrix_ = ortho(0.0f, 1280.0f, 0.0f, 720.0f, -1.0f, 1.0f);
    mat4 projectionMatrix = perspective(radians(cameraAngle_), aspectRatio_,
        nearClipping_, farClipping_);
    mat4 viewMatrix(1.0f);
    vpMatrixOrthographic_ = orthographicMatrix_;
    if (target_ != nullptr) {
        /* If there is a target, aim the camera at it */
        eye = target_->getPosition(offset_);
        center = target_->getPosition(center);
        /* Sprite Objects will use a simple translation matrix instead of a view matrix */
        if (target_->type() == SPRITE_OBJECT) {
            viewMatrix = glm::translate(glm::mat4(1.0f), vec3(-1) * (target_->getPosition() - initialTargetPos_));
            vpMatrixOrthographic_ = orthographicMatrix_ * viewMatrix;
        } else {
            viewMatrix = lookAt(eye, center, vec3(0, 1, 0));
        }
    }
    vpMatrixPerspective_ = projectionMatrix * viewMatrix;
}

void CameraObject::update() {
    // Update aspect ratio
    setAspectRatio(resolution_.x / resolution_.y);
    render();
    // Higher priority object renders are deferred
    vector<SceneObject *> lowDefer;
    vector<SceneObject *> mediumDefer;
    vector<SceneObject *> highDefer;
    for (auto it = sceneObjects_.begin(); it != sceneObjects_.end(); ++it) {
        // Send the current screen res to each object
        /// @todo Maybe use a global variable for resolution?
        (*it)->setResolution(resolution_);
        // Check if the object is ORTHO or PERSPECTIVE
        switch ((*it)->type()) {
            case GAME_OBJECT:
                (*it)->setVpMatrix(vpMatrixPerspective_);
                break;
            case UI_OBJECT:
                /* Do not apply view to UI */
                (*it)->setVpMatrix(orthographicMatrix_);
                break;
            case SPRITE_OBJECT:
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
        switch ((*it)->getRenderPriority()) {
            case RenderPriority::HIGH:
                highDefer.push_back(*it);
                break;
            case RenderPriority::MEDIUM:
                mediumDefer.push_back(*it);
                break;
            case RenderPriority::LOW:
                lowDefer.push_back(*it);
                break;
            default:
                fprintf(stderr, "CameraObject::update: Bad render priority received! %d\n",
                    (*it)->getRenderPriority());
                assert(0);
                break;
        }
    }
    auto renderLoop = [](vector<SceneObject *> objs) {
        for (auto obj : objs) {
            obj->update();
        }
    };
    // Render low priority objects first
    renderLoop(lowDefer);
    // Then medium...
    renderLoop(mediumDefer);
    // Then high on top of all of those
    renderLoop(highDefer);
}

void CameraObject::addSceneObject(SceneObject *sceneObject) {
    sceneObjects_.push_back(sceneObject);
}

void CameraObject::removeSceneObject(string objectName) {
    auto objectIt = std::find_if(sceneObjects_.begin(), sceneObjects_.end(),
        [objectName](SceneObject *obj) {
            return obj->getObjectName().compare(objectName) == 0;
        });

    sceneObjects_.erase(objectIt);
}
