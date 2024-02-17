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
#include <SceneObject.hpp>
#include <Polygon.hpp>
#include <GfxController.hpp>
#include <common.hpp>

class ColliderObject : public SceneObject {
 public:
    ColliderObject(vec4 offset, vec4 minPoints, vec4 center, vec4 originalCenter,
        Polygon *poly, GfxController *gfxController);
 private:
    vec4 offset_;
    vec4 minPoints_;
    vec4 center_;
    vec4 originalCenter_;
    Polygon *poly_;
};
