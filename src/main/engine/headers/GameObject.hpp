#pragma once
#include "common.hpp"
#include "ModelImport.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H

class GameObject {
    public:
        /**
         * @brief Sets the rotation of the GameObject
         * 
         * @param rotation to set on the GameObject
         */
        virtual void setRotation(vec3 rotation) {
            rot = rotation;
        }

        /**
         * @brief Sets the rendering mode for the GameObject
         * 
         * @param mode for rendering GameObject; perspective or orthographic
         */
        virtual void setViewMode(ViewMode mode) {
            viewMode = mode;
        }


    private:
        polygon *model;
        mat4 translateMatrix, scaleMatrix, rotateMatrix; // Model Matrix
        mat4 vpMatrix; // Projection  *View matrix
        GLuint rotateID, scaleID, translateID, vpID, textureID, textCoordsID,
            hasTextureID, directionalLightID, luminanceID, rollOffID, programID,
            MVPID, collider_shaderID;
        GLint textureCoordID, uniform_mytexture;
        vector<GLint> hasTexture;
        vec3 pos, rot, vel; // Position, rotation and velocity 3D vectors
        GLfloat scale;
        bool configured;
        ViewMode viewMode;
        int currentCamera;
        vec3 directionalLight;
        GLfloat luminance, rollOff;
        mutex infoLock;
        unsigned int VAO;
        colliderInfo collider;
}