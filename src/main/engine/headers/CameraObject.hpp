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
#include <ModelImport.hpp>
#include <CameraObjectStructs.hpp>

class CameraObject: public SceneObject {
 public:
        // Constructors
        explicit CameraObject(cameraInfo camInfo);
        ~CameraObject();

        // Setters
        inline void setOffset(vec3 offset) { this->offset = offset; }
        inline void setTarget(GameObject *target) { this->target = target; }
        inline void setAspectRatio(GLfloat aspectRatio) { this->aspectRatio = aspectRatio; }

        // Getters
        inline vec3 getOffset() { return this->offset; }
        inline GameObject *getTarget() { return this->target; }  /// @todo: Use reference instead of pointer here
        inline GLfloat getAspectRatio() { return this->aspectRatio; }

        // Camera Specific Methods
        void render() override;

 private:
        GameObject *target;

        vec3 offset;

        GLfloat aspectRatio;
        GLfloat nearClipping;
        GLfloat farClipping;
        GLfloat cameraAngle;
};
