#include <TestObject.hpp>

TestObject::TestObject() : SceneObject(ObjectType::UNDEFINED, TEST_OBJECT_NAME, nullptr) {
}

TestObject::TestObject(string name) : SceneObject(ObjectType::UNDEFINED, name, nullptr) {
}

void TestObject::render() {
}

void TestObject::update() {
}
