/**
 * @file BaseObject.cpp
 * @author Christian Galvez
 * @brief Implementation of an almost bare SceneObject.
 * @date 02-23-26
 * @copyright Studious Engine 2026
 */

#include <BaseObject.hpp>

BaseObject::BaseObject(string objectName, std::function<void(SceneObject *)> func) :
    SceneObject(func, ObjectType::UNDEFINED, objectName, nullptr) {
}
