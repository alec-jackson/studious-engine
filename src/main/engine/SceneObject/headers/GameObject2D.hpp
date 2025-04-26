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
#include <memory>
#include <SceneObject.hpp>
#include <ColliderObject.hpp>
#include <TrackExt.hpp>

class GameObject2D : public SceneObject, public TrackExt {
 public:
    // Constructors
    /// @todo Remove ObjectType - we render by camera now, so this isn't really needed...
    explicit GameObject2D(string texturePath, vec3 position, float scale, unsigned int programId,
        string objectName, ObjectType type, ObjectAnchor anchor, GfxController *gfxController);
    ~GameObject2D() override;

    // Getter methods
    ColliderObject *getCollider();

    // Render method
    void render() override;
    void update() override;
    void initializeTextureData();
    virtual void initializeShaderVars() = 0;
    void initializeVertexData();
    void createCollider(int programId);
    void setDimensions(int width, int height);

    // Animation Methods
    virtual void createAnimation(int width, int height, int frameCount) = 0;

 protected:
    string texturePath_;
    std::shared_ptr<ColliderObject> collider_;
    vector<float> vertTexData_;

    unsigned int textureId_;
    unsigned int modelMatId_;
    unsigned int projectionId_;
    unsigned int tintId_;

    unsigned int vao_;
    unsigned int vbo_;

    unsigned int textureWidth_;
    unsigned int textureHeight_;

    ObjectAnchor anchor_;

    mat4 modelMat_;
};
