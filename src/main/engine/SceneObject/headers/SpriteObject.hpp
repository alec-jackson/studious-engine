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
#include <GameObject2D.hpp>
#include <ColliderObject.hpp>

class SpriteObject : public GameObject2D {
 public:
    // Constructors
    /// @todo Remove ObjectType - we render by camera now, so this isn't really needed...
    explicit SpriteObject(string spritePath, vec3 position, float scale, unsigned int programId,
        string objectName, ObjectType type, ObjectAnchor anchor, GfxController *gfxController);
    ~SpriteObject() override;
    void initializeShaderVars();

    void render() override;
    void update() override;

    // Getters
    inline vec3 getTint() { return tint_; }

    // Setters
    inline void setTint(vec3 tint) { tint_ = tint; }

 private:
    vec3 tint_;
};
