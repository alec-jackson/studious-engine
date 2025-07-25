/**
 * @file CameraObject.hpp
 * @author Christian Galvez
 * @brief CameraObject class is a SceneObject; used for rendering GameObjects
 * @version 0.1
 * @date 2023-07-28
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once
#include <vector>
#include <string>
#include <memory>
#include <common.hpp>
#include <GameObject.hpp>

class CameraObject : public SceneObject {
 public:
    // Constructors
    explicit CameraObject(SceneObject *target, vec3 offset, float cameraAngle, float aspectRatio,
        float nearClipping, float farClipping, ObjectType type, string objectName, GfxController *gfxController);
    ~CameraObject();

    // Setters
    inline void setOffset(vec3 offset) { offset_ = offset; }
    inline void setAspectRatio(float aspectRatio) { aspectRatio_ = aspectRatio; }
    inline void setTarget(SceneObject *target) { target_ = target; }

    // Getters
    inline vec3 getOffset() { return offset_; }
    inline SceneObject *getTarget() { return target_; }
    inline float getAspectRatio() { return aspectRatio_; }
    inline vector<SceneObject *> getSceneObjects() const { return sceneObjects_; }

    // Camera Specific Methods
    void render() override;
    void update() override;
    void addSceneObject(SceneObject *gameObject);
    void removeSceneObject(string objectName);
    void resetRenderPriorityMap();
    inline void clearSceneObjects() { sceneObjects_.clear(); sceneObjectsOrdered_.clear(); }

 private:
    void resetRenderPriorityMap_();
    SceneObject *target_;
    vector<SceneObject *> sceneObjects_;
    // Render priority to list of scene objects
    map<uint, vector<SceneObject *>> sceneObjectsOrdered_;
    vec3 offset_;
    vec3 initialTargetPos_;
    mat4 vpMatrixPerspective_;
    mat4 vpMatrixOrthographic_;
    mat4 orthographicMatrix_;

    float cameraAngle_;
    float aspectRatio_;
    float nearClipping_;
    float farClipping_;
};
