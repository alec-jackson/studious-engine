/**
 * @file ColliderObject.hpp
 * @author Christian Galvez
 * @brief 
 * @version 0.1
 * @date 2024-02-15
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once
#include <vector>
#include <string>
#include <Polygon.hpp>
#include <SceneObject.hpp>
#include <GfxController.hpp>
#include <common.hpp>

class ColliderObject : public SceneObject {
 public:
    ColliderObject(Polygon *target, unsigned int programId, const mat4 &translateMatrix, const mat4 &scaleMatrix,
        const mat4 &vpMatrix, ObjectType type, string objectName, GfxController *gfxController);
    void updateCollider();
    void render() override;
    void update() override;
    void createCollider(unsigned int programId);
    int getCollision(ColliderObject *object, vec3 moving);
    float getColliderVertices(vector<float> vertices, int axis, bool (*test)(float a, float b));
    inline vec4 center() { return center_; }
    inline vec4 offset() { return offset_; }
    ~ColliderObject();

 private:
    vec4 offset_;
    vec4 minPoints_;
    vec4 center_;
    vec4 originalCenter_;
    Polygon *poly_ = nullptr;
    Polygon *target_;
    // Create references to translate/scale matrices
    const mat4 &translateMatrix_;
    const mat4 &scaleMatrix_;
    const mat4 &vpMatrix_;
    int mvpId_;
};

