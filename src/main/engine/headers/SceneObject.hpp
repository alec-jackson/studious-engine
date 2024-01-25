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

class SceneObject {
 public:
        // Constructors
        inline explicit SceneObject(vec3 position, vec3 rotation, string objectName, GLfloat scale, GLuint programId,
              const GfxController &gfxController):
              position(position), rotation(rotation), objectName(objectName), scale(scale), programId(programId),
              gfxController_ { gfxController } {}
        inline explicit SceneObject(const GfxController &gfxController): gfxController_ { gfxController } {}
        // Setter methods
        inline void setVpMatrix(mat4 vpMatrix) { this->vpMatrix = vpMatrix; }
        inline void setPosition(vec3 position) { this->position = position; }
        inline void setRotation(vec3 rotation) { this->rotation = rotation; }
        inline void setScale(GLfloat scale) { this->scale = scale ; }

        // Getter methods
        inline mat4 getVpMatrix() { return this->vpMatrix; }
        inline vec3 getPosition() { return this->position; }
        inline vec3 getPosition(vec3 offset) { return this->position + offset; }
        inline vec3 getRotation() { return this->rotation; }
        inline string getObjectName() { return this->objectName; }

        virtual void render() = 0;

 protected:
        mat4 translateMatrix;
        mat4 scaleMatrix;
        mat4 rotateMatrix;
        mat4 vpMatrix;

        vec3 position;
        vec3 rotation;

        string objectName;
        GLfloat scale;
        GLuint programId;

        const GfxController &gfxController_;
};
