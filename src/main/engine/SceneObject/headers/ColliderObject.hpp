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
#include <memory>
#include <Polygon.hpp>
#include <SceneObject.hpp>
#include <GfxController.hpp>
#include <common.hpp>

class ColliderObject : public SceneObject {
 public:
    ColliderObject(Polygon *target, unsigned int programId, mat4 *translateMatrix, mat4 *scaleMatrix,
        mat4 *vpMatrix, ObjectType type, string objectName, GfxController *gfxController);
    ColliderObject(const vector<float> &vertTexData, unsigned int programId, mat4 *translateMatrix, mat4 *scaleMatrix,
        mat4 *vpMatrix, ObjectType type, string objectName, GfxController *gfxController);
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
    std::shared_ptr<Polygon> poly_;
    Polygon *target_;
    // Use pointers to the parent object's transform matrices
    mat4 *pTranslateMatrix_;
    mat4 *pScaleMatrix_;
    mat4 *pVpMatrix_;
    int mvpId_;
};

