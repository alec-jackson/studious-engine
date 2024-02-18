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
#include <string>
#include <vector>
#include <ModelImport.hpp>
#include <SceneObject.hpp>
#include <ColliderObject.hpp>

class GameObject: public SceneObject {
 public:
        // Constructurs
        explicit GameObject(Polygon *characterModel, vec3 position, vec3 rotation, GLfloat scale, int camera,
              string objectName, GfxController *gfxController);
        ~GameObject();

        // Setters
        inline void setViewMode(ViewMode viewMode) { this->viewMode = viewMode; }
        inline void setScale(GLfloat scale) { this->scale = scale; }
        inline void setDirectionalLight(vec3 directionalLight) { this->directionalLight = directionalLight; }
        inline void setLuminance(GLfloat luminance) { this->luminance = luminance; }
        inline void setProgramId(GLuint programId) { this->programId = programId; }

        // Getters
        inline ViewMode getViewMode() { return this->viewMode; }
        inline GLfloat getScale() { return this->scale; }
        inline vec3 getDirectionalLight() { return this->directionalLight; }
        inline GLfloat getLuminance() { return this->luminance; }
        inline GLuint getProgramId() { return this->programId; }
        inline int getCameraId() { return this->cameraId; }

        // Special Getters
        inline Polygon *getModel() { return model; }
        ColliderObject *getCollider();
        GLfloat getColliderVertices(vector<GLfloat> vertices, int axis, bool (*test)(float a, float b));

        // Other methods
        void deleteTextures();  /// @todo: DEPRECATED - Use destructor for this now...
        void createCollider(int programId);

        void render() override;

 private:
        Polygon *model;  // Change this to a proper class at some point

        int cameraId;  /// @todo: Why is this managed in GameObject?
        unsigned int VAO;  /// @todo: Why do we have this?

        GLuint vpId, modelId, textureId, textCoordsId,
            hasTextureId, directionalLightId, luminanceId, rollOffId,
            collider_shaderId;  /// @todo: Organize these into another class

        GLint textureCoordId;

        GLfloat luminance;
        GLfloat rollOff;

        vector<GLint> hasTexture;
        vec3 directionalLight;
        vec3 velocity;

        ViewMode viewMode;  /// @todo Add this to GameCamera refactor
        ColliderObject *collider_ = nullptr;
};
