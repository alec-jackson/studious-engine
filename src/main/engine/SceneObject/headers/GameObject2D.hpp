/**
 * @file SpriteObject.hpp
 * @author Christian Galvez
 * @brief SpriteObject is a SceneObject; can be rendered by a CameraObject
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once
#include <string>
#include <vector>
#include <SceneObject.hpp>
#include <ColliderObject.hpp>

class GameObject2D : public SceneObject {
 public:
    // Constructors
    /// @todo Remove ObjectType - we render by camera now, so this isn't really needed...
    explicit GameObject2D(string texturePath, vec3 position, float scale, unsigned int programId,
        string objectName, ObjectType type, ObjectAnchor anchor, GfxController *gfxController);
    ~GameObject2D() override;

    // Render method
    void render() override;
    void update() override;
    void initializeTextureData();
    void initializeShaderVars();
    void initializeVertexData();

 protected:
    string texturePath_;

    unsigned int textureId_;
    unsigned int modelMatId_;
    unsigned int tintId_;

    unsigned int vao_;
    unsigned int vbo_;

    unsigned int textureWidth_;
    unsigned int textureHeight_;

    ObjectAnchor anchor_;

    mat4 modelMat_;
};
