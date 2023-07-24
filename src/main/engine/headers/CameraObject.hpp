#pragma once
#include "common.hpp"
#include "ModelImport.hpp"
#include "CameraObjectStructs.hpp"

class CameraObject: public SceneObject {
    public:
        // Constructors
        CameraObject(cameraInfo camInfo);
        ~CameraObject();

        // Setters
        inline void setOffset(vec3 offset) { this->offset = offset; };
        inline void setTarget(GameObject *target) { this->target = target; };
        inline void setAspectRatio(GLfloat aspectRatio) { this->aspectRatio = aspectRatio; };

        // Getters
        inline vec3 getOffset() { return this->offset; };
        inline GameObject *getTarget() { return this->target; }; // TODO: Reference here
        inline GLfloat getAspectRatio() { return this->aspectRatio; };

        // Camera Specific Methods
        void updateCamera();
    private:
        GameObject *target;

        vec3 offset;

        GLfloat aspectRatio;
        GLfloat nearClipping;
        GLfloat farClipping;
        GLfloat cameraAngle;
};
