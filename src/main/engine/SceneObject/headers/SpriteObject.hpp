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

enum SpriteAnchor {
    CENTER,
    BOTTOM_LEFT
};

class SpriteObject : public SceneObject {
 public:
    // Constructors
    /// @todo Remove ObjectType - we render by camera now, so this isn't really needed...
    explicit SpriteObject(string spritePath, vec3 position, float scale, unsigned int programId,
        string objectName, ObjectType type, SpriteAnchor anchor, GfxController *gfxController);
    ~SpriteObject() override;

    // Render method
    void render() override;
    void update() override;
    void initializeSprite();
    void initializeShaderVars();

    // Getters
    inline vec3 getTint() { return tint_; }

    // Setters
    inline void setTint(vec3 tint) { tint_ = tint; }

 private:
    string spritePath_;

    unsigned int textureId_;
    unsigned int modelMatId_;
    unsigned int tintId_;

    unsigned int vao_;
    unsigned int vbo_;

    SpriteAnchor anchor_;

    mat4 modelMat_;
    vec3 tint_;
};
