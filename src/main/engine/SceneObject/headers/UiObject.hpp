/**
 * @file UiObject.hpp
 * @author Christian Galvez
 * @brief UiObject is a SceneObject; can be rendered by a CameraObject
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

class UiObject : public SceneObject {
 public:
    // Constructors
    /// @todo Remove ObjectType - we render by camera now, so this isn't really needed...
    explicit UiObject(string spritePath, vec3 position, float scale, unsigned int programId,
        string objectName, ObjectType type, GfxController *gfxController);
    ~UiObject() override;

    // Render method
    void render() override;
    void update() override;
    void initializeSprite();
    void initializeShaderVars();
    float *generateVertices(float x, float y, float iFx, float iFy);
    void generateVertexBase(float *vertexData, int triIdx, float x, float y, float x2, float y2);

 private:
    string spritePath_;
    unsigned int textureId_;
    unsigned int vao_;
    unsigned int vbo_;
    float *vertexData_;
};
