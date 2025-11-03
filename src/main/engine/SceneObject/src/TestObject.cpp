/**
 * @file TestObject.cpp
 * @author Christian Galvez
 * @brief Implementation for TestObject class. Add whatever you want for testing here...
 * @version 0.1
 * @date 2025-07-19
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <TestObject.hpp>
#include <string>
#include <memory>
#include <DummyGfxController.hpp>

DummyGfxController dummyGfx;

TestObject::TestObject() : SceneObject(ObjectType::UNDEFINED, TEST_OBJECT_NAME, nullptr) {
}

TestObject::TestObject(string name) : SceneObject(ObjectType::UNDEFINED, name, nullptr) {
}

TestObject::TestObject(std::shared_ptr<Polygon> polygon, string name) :
    SceneObject(ObjectType::UNDEFINED, name, &dummyGfx), polygon_ { polygon }
{
    scale_ = 1.0f;
}

void TestObject::render() {
    VISIBILITY_CHECK;
}

void TestObject::update() {
    SceneObject::updateModelMatrices();
}

void TestObject::createCollider() {
    collider_ = std::make_shared<ColliderObject>(polygon_, UINT_MAX, this);
}
