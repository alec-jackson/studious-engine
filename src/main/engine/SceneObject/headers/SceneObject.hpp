/**
 * @author Christian Galvez
 * @date 2023-07-23
 * @brief This is used as the base class for all objects used in GameObject scenes (GameObject, TextObject,
 *        CameraObject, etc). This base class should only contain methods and member variables that are common across
 *        these objects. This class is not designed to be used directly for anything.
 * @copyright Copyright (c) 2023
 */
#pragma once
#include <string>
#include <common.hpp>
#include <GfxController.hpp>

class SceneObject {
 public:
    // Constructors
    inline explicit SceneObject(vec3 position, vec3 rotation, string objectName, GLfloat scale, GLuint programId,
            GfxController *gfxController):
            position(position), rotation(rotation), objectName(objectName), scale(scale), programId(programId),
            gfxController_ { gfxController } {}
    inline explicit SceneObject(GfxController *gfxController): gfxController_ { gfxController } {}
    // Setter methods
    inline void setVpMatrix(mat4 vpMatrix) { vpMatrix_ = vpMatrix; }
    inline void setPosition(vec3 position) { this->position = position; }
    inline void setRotation(vec3 rotation) { this->rotation = rotation; }
    inline void setScale(GLfloat scale) { this->scale = scale ; }

    // Getter methods
    inline const mat4 &vpMatrix() const { return vpMatrix_; }
    inline const mat4 &rotateMatrix() const { return rotateMatrix_; }
    inline const mat4 &translateMatrix() const { return translateMatrix_; }
    inline const mat4 &scaleMatrix() const { return scaleMatrix_; }
    inline vec3 getPosition() const { return this->position; }
    inline vec3 getPosition(vec3 offset) const { return this->position + offset; }
    inline vec3 getRotation() const { return this->rotation; }
    inline string getObjectName() const { return this->objectName; }

    virtual void render() = 0;

 protected:
    mat4 translateMatrix_;
    mat4 scaleMatrix_;
    mat4 rotateMatrix_;
    mat4 vpMatrix_;

    vec3 position;
    vec3 rotation;

    string objectName;
    GLfloat scale;
    GLuint programId;

    GfxController *gfxController_;
};
