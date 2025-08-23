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

// Include External Libaries
#include <string>

// Include Internal Headers
#include <TestObject.hpp>

TestObject::TestObject() : SceneObject(ObjectType::UNDEFINED, TEST_OBJECT_NAME, nullptr) {
}

TestObject::TestObject(string name) : SceneObject(ObjectType::UNDEFINED, name, nullptr) {
}

void TestObject::render() {
}

void TestObject::update() {
}
