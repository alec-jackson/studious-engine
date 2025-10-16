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
#include <atomic>
#include <Polygon.hpp>
#include <SceneObject.hpp>
#include <GfxController.hpp>
#include <common.hpp>

class ColliderObject : public SceneObject {
 public:
    ColliderObject(std::shared_ptr<Polygon> target, uint programId, SceneObject *owner);
    ColliderObject(const vector<float> &vertTexData, uint programId, SceneObject *owner);
    void updateCollider();
    void render() override;
    void update() override;
    void createCollider();
    int getCollision(ColliderObject *object, vec3 moving);
    float getColliderVertices(vector<float> vertices, int axis, bool (*test)(float a, float b));
    inline vec4 center() { return center_; }
    inline vec4 offset() { return offset_; }
    ~ColliderObject();
    inline static void setDrawCollider(bool enable) { drawCollider_ = enable; }
    inline static bool getDrawCollider() { return drawCollider_; }
    vec3 getEdgePoint(ColliderObject *object, vec3 velocity);

 private:
    vec4 offset_;
    vec4 minPoints_;
    vec4 center_;
    vec4 originalCenter_;
    std::shared_ptr<Polygon> poly_;
    std::shared_ptr<Polygon> target_;
    // Use references to the parent object's transform matrices
    const mat4 &pTranslateMatrix_;
    const mat4 &pScaleMatrix_;
    const mat4 &pVpMatrix_;
    int mvpId_;
    inline static std::atomic<bool> drawCollider_;
};
