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
#include <common.hpp>
#include <ModelImport.hpp> // Might not be needed
#include <GameObject.hpp>

class CameraObject : public SceneObject {
 public:
    // Constructors
    explicit CameraObject(GameObject *target, vec3 offset, GLfloat cameraAngle, GLfloat aspectRatio,
    GLfloat nearClipping, GLfloat farClipping, GfxController *gfxController);
    ~CameraObject();

    // Setters
    inline void setOffset(vec3 offset) { offset_ = offset; }
    inline void setAspectRatio(GLfloat aspectRatio) { aspectRatio_ = aspectRatio; }

    // Getters
    inline vec3 getOffset() { return offset_; }
    inline GameObject *getTarget() { return target_; }
    inline GLfloat getAspectRatio() { return aspectRatio_; }

    // Camera Specific Methods
    void render() override;

 private:
    GameObject *target_;

    vec3 offset_;

    GLfloat aspectRatio_;
    GLfloat nearClipping_;
    GLfloat farClipping_;
    GLfloat cameraAngle_;
};
