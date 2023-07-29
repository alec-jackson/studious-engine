/**
 * @file GameObject.hpp
 * @author Christian Galvez
 * @brief GameObject is a SceneObject; can be rendered by a CameraObject
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once
#include "common.hpp"
#include "ModelImport.hpp"
#include "SceneObject.hpp"
#include "GameObjectStructs.hpp"

class GameObject: public SceneObject {
    public:
        // Constructurs
        GameObject(gameObjectInfo objectInfo);
        GameObject();
        ~GameObject();

        // Setters
        inline void setViewMode(ViewMode viewMode) { this->viewMode = viewMode; };
        inline void setScale(GLfloat scale) { this->scale = scale; };
        inline void setDirectionalLight(vec3 directionalLight) { this->directionalLight = directionalLight; };
        inline void setLuminance(GLfloat luminance) { this->luminance = luminance; };
        inline void setProgramId(GLuint programId) { this->programId = programId; };
        
        // Getters
        inline ViewMode getViewMode() { return this->viewMode; };
        inline GLfloat getScale() { return this->scale; };
        inline vec3 getDirectionalLight() { return this->directionalLight; };
        inline GLfloat getLuminance() { return this->luminance; };
        inline GLuint getProgramId() { return this->programId; };
        inline int getCameraId() { return this->cameraId; };

        // Special Getters
        polygon *getModel(); /// @todo: This should return a reference
        colliderInfo getCollider();
        GLfloat getColliderVertices(vector<GLfloat> vertices, int axis, bool (*test)(float a, float b));
        string getCollisionTag();

        // Special Setters
        void setCollisionTag(string collisionTag);

        // Render method (previously called drawShape)
        void render();

        // Other methods
        void deleteTextures(); /// @todo: DEPRECATED - Use destructor for this now...
        int createCollider(int programId);

    private:
        polygon *model; // Change this to a proper class at some point

        int cameraId; /// @todo: Why is this managed in GameObject?
        unsigned int VAO; /// @todo: Why do we have this?

        GLuint rotateId, scaleId, translateId, vpId, textureId, textCoordsId,
            hasTextureId, directionalLightId, luminanceId, rollOffId,
            mvpId, collider_shaderId; /// @todo: Organize these into another class

        GLint textureCoordId; 

        GLfloat luminance;
        GLfloat rollOff;

        vector<GLint> hasTexture;
        vec3 directionalLight;

        ViewMode viewMode; 
    protected:
        GLfloat scale;
        GLuint programId;
        colliderInfo collider; /// @todo: Refactor colliderInfo
};
