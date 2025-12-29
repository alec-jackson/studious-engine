/**
 * @file TPSCameraObject.cpp
 * @author Christian Galvez
 * @copyright Studious Engine 2025
 * @date 2025-12-28
 * @brief Implementation for the third person camera object class.
 */
#include <TPSCameraObject.hpp>
#include <memory>
#include <glm/geometric.hpp>
#include <InputController.hpp>
#include <AnimationController.hpp>
#include <physics.hpp>

extern std::unique_ptr<InputController> inputController;
extern std::unique_ptr<AnimationController> animationController;
extern std::unique_ptr<PhysicsController> physicsController;

#define INVERT_MODIFIER(flag) if (flag) modifier *= -1.0f

TPSCameraObject::TPSCameraObject(SceneObject *target, vec3 offset, float cameraAngle, float aspectRatio,
    float nearClipping, float farClipping, ObjectType type, string objectName, GfxController *gfxController) :
    ComplexCameraObject(target, offset, cameraAngle, aspectRatio, nearClipping, farClipping, type, objectName,
    gfxController) {
    invertX = false;
    invertY = false;
}
