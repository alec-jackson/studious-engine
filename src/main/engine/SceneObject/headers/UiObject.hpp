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
#include <memory>
#include <GameObject2D.hpp>
#include <ColliderObject.hpp>

#define POINTS_PER_TRIANGLE 3
#define TRIANGLES_PER_QUAD 2
#define QUADS_PER_UI_ELEM 9

class UiObject : public GameObject2D {
 public:
    // Constructors
    /// @todo Remove ObjectType - we render by camera now, so this isn't really needed...
    explicit UiObject(string spritePath, vec3 position, float scale, float wScale, float hScale, unsigned int programId,
        string objectName, ObjectType type, ObjectAnchor anchor, GfxController *gfxController);
    ~UiObject() override;

    // Render method
    void render() override;
    void update() override;
    void initializeShaderVars() override;
    void initializeVertexData();
    std::shared_ptr<float[]> generateVertices(float x, float y, float iFx, float iFy);
    void generateVertexBase(std::shared_ptr<float[]> vertexData, int triIdx, float x, float y, float x2, float y2);

    // Set scale methods
    void setHStretch(float wScale);
    void setWStretch(float hScale);

    // Get scale methods
    vec3 getStretch();

    // Animation functions
    void createAnimation(int width, int height, int frameCount) override;

 private:
    unsigned int wScaleId_;
    unsigned int hScaleId_;
    unsigned int vertexIndexId_;
    unsigned int vertexIndexVbo_;

    std::shared_ptr<float[]> vertexData_;
    std::shared_ptr<float[]> vertexIndexData_;

    float wScale_;
    float hScale_;
};
