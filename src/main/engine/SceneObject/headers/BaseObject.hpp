/**
 * @file BaseObject.hpp
 * @brief Bare SceneObject implementation that can be used as a
 * blank container for whatever.
 * @author Christian Galvez
 * @date 02-23-26
 * @copyright Studious Engine 2026
 */

#pragma once
#include <SceneObject.hpp>

class BaseObject : public SceneObject {
 public:
    BaseObject(string objectName, std::function<void(SceneObject *)> func);
    inline void update() {}
    inline void render() {}
};
