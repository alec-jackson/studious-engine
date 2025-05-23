/**
 * @author Christian Galvez
 * @date 2023-07-23
 * @brief This is used as the base class for all objects used in GameObject scenes (GameObject, TextObject,
 *        CameraObject, etc). This base class should only contain methods and member variables that are common across
 *        these objects. This class is not designed to be used directly for anything.
 * @copyright Copyright (c) 2023
 */
#pragma once
#include <string>
#include <common.hpp>
#include <GfxController.hpp>

/* Define constants for shader names */
#define UIOBJECT_PROG_NAME "uiObject"
#define SPRITEOBJECT_PROG_NAME "spriteObject"
#define GAMEOBJECT_PROG_NAME "gameObject"
#define TEXTOBJECT_PROG_NAME "textObject"
#define COLLIDEROBJECT_PROG_NAME "colliderObject"
#define TILEOBJECT_PROG_NAME "tileObject"

/// @todo Add SceneObject grouping for shared model changes
enum ObjectType {
    UNDEFINED,
    TEXT_OBJECT,
    CAMERA_OBJECT,
    GAME_OBJECT,
    UI_OBJECT,
    SPRITE_OBJECT,
    TILE_OBJECT
};

enum RenderPriority {
    HIGH,
    MEDIUM,
    LOW
};

enum ObjectAnchor {
    CENTER,
    BOTTOM_LEFT,
    TOP_LEFT
};

class SceneObject {
 public:
    // Constructors
    inline explicit SceneObject(vec3 position, vec3 rotation, string objectName, float scale, unsigned int programId,
        ObjectType type, GfxController *gfxController):
            position(position), rotation(rotation), objectName(objectName), scale_(scale), programId_(programId),
            type_ { type }, gfxController_ { gfxController } {}
    inline explicit SceneObject(ObjectType type, string objectName, GfxController *gfxController):
        objectName { objectName }, type_ { type }, gfxController_ { gfxController } {}
    virtual ~SceneObject() = default;
    // Setter methods
    inline void setVpMatrix(mat4 vpMatrix) { vpMatrix_ = vpMatrix; }
    inline void setPosition(vec3 position) { this->position = position; }
    inline void setRotation(vec3 rotation) { this->rotation = rotation; }
    inline void setResolution(vec3 resolution) { this->resolution_ = resolution; }
    inline void setScale(float scale) { this->scale_ = scale ; }
    inline void setRenderPriority(RenderPriority renderPriority) { this->renderPriority_ = renderPriority; }

    // Getter methods
    inline const mat4 &vpMatrix() const { return vpMatrix_; }
    inline const mat4 &rotateMatrix() const { return rotateMatrix_; }
    inline const mat4 &translateMatrix() const { return translateMatrix_; }
    inline const mat4 &scaleMatrix() const { return scaleMatrix_; }
    inline vec3 getPosition() const { return this->position; }
    inline vec3 getPosition(vec3 offset) const { return this->position + offset; }
    inline vec3 getRotation() const { return this->rotation; }
    inline float getScale() const { return this->scale_; }
    inline RenderPriority getRenderPriority() const { return this->renderPriority_; }
    inline vec3 getResolution() const { return this->resolution_; }
    inline string getObjectName() const { return this->objectName; }
    inline ObjectType type() const { return type_; }

    // Interface methods
    virtual void render() = 0;
    virtual void update() = 0;

 protected:
    mat4 translateMatrix_;
    mat4 scaleMatrix_;
    mat4 rotateMatrix_;
    mat4 vpMatrix_;

    vec3 position;
    vec3 rotation;
    vec3 resolution_;

    const string objectName;
    float scale_;
    unsigned int programId_;
    unsigned int vao_;
    ObjectType type_;

    RenderPriority renderPriority_ = RenderPriority::HIGH;

    GfxController *gfxController_;
};
