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

#define X_MATCH 1
#define Y_MATCH 2
#define Z_MATCH 4
#define ALL_MATCH 7
#define NO_MATCH 0


class ColliderObject : public SceneObject {
 public:
    ColliderObject(std::shared_ptr<Polygon> target, uint programId, SceneObject *owner);
    ColliderObject(const vector<float> &vertTexData, uint programId, SceneObject *owner);
    void updateCollider();
    void render() override;
    void update() override;
    void createCollider();
    static vec4 createCenter(const mat4 &tm, const mat4 &sm, ColliderObject *col);
    int getCollision(ColliderObject *object);
    static int getCollisionRaw(vec3 p1, ColliderObject *c1, vec3 p2, ColliderObject *c2);
    float getColliderVertices(vector<float> vertices, int axis, bool (*test)(float a, float b));
    inline vec4 center() { return center_; }
    inline vec4 offset() { return offset_; }
    ~ColliderObject();
    inline static void setDrawCollider(bool enable) { drawCollider_ = enable; }
    inline static bool getDrawCollider() { return drawCollider_; }
    vec3 getEdgePoint(ColliderObject *object, bool bothKin);
    inline const mat4 &pScaleMatrix() { return pScaleMatrix_; }
    inline const vec4 &minPoints() { return minPoints_; }
    inline const vec4 &originalCenter() { return originalCenter_; }
    static int getCollisionRaw(vec3 p1, ColliderObject *c1, vec3 p2, ColliderObject *c2);
    static vec4 createOffset(const mat4 &tm, const mat4 &sm, const vec4 &center, ColliderObject *col);

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
    const vec3 &pPos_;
    int mvpId_;
    inline static std::atomic<bool> drawCollider_;
};
