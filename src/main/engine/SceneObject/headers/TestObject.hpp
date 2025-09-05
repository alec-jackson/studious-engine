/**
 * @file TestObject.hpp
 * @author Christian Galvez
 * @brief Basic implementation of a SceneObject for testing.
 * @version 0.1
 * @date 2025-07-19
 *
 * @copyright Copyright (c) 2025
 *
 */
#pragma once
#include <set>
#include <string>
#include <SceneObject.hpp>

using std::set;

#define TEST_OBJECT_NAME "testObject"
#define PARENT_OBJECT_NAME "parentObject"

class TestObject : public SceneObject {
 public:
    TestObject();
    explicit TestObject(string name);
    void render() override;
    void update() override;
    inline SceneObject *getParent() { return parent_; }
    inline const set<SceneObject *> &getChildren() { return children_; }
    inline const mat4 &getTranslationMatrix() { return translateMatrix_; }
    inline const mat4 &getRotationMatrix() { return rotateMatrix_; }
    inline const mat4 &getScaleMatrix() { return scaleMatrix_; }
};
