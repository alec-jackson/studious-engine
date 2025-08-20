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
#include <set>
#include <common.hpp>
#include <GfxController.hpp>

/* Define constants for shader names */
#define UIOBJECT_PROG_NAME "uiObject"
#define SPRITEOBJECT_PROG_NAME "spriteObject"
#define GAMEOBJECT_PROG_NAME "gameObject"
#define TEXTOBJECT_PROG_NAME "textObject"
#define COLLIDEROBJECT_PROG_NAME "colliderObject"
#define TILEOBJECT_PROG_NAME "tileObject"

/* Default Render Priority Levels */
#define RENDER_PRIOR_LOWEST 0
#define RENDER_PRIOR_LOW 10
#define RENDER_PRIOR_MEDIUM 20
#define RENDER_PRIOR_HIGH 40
#define RENDER_PRIOR_HIGHEST 100

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
        position { 0 }, rotation { 0 }, objectName { objectName }, scale_ { 0 }, type_ { type }, gfxController_ { gfxController } {}
    virtual ~SceneObject();
    // Setter methods
    inline void setVpMatrix(mat4 vpMatrix) { vpMatrix_ = vpMatrix; }
    inline void setPosition(vec3 position) { this->position = position; }
    inline void setRotation(vec3 rotation) { this->rotation = rotation; }
    inline void setResolution(vec3 resolution) { this->resolution_ = resolution; }
    inline void setScale(float scale) { this->scale_ = scale ; }
    inline void setRenderPriority(uint renderPriority) { this->renderPriority_ =
        renderPriority <= RENDER_PRIOR_HIGHEST ? renderPriority : RENDER_PRIOR_HIGHEST; }

    // Getter methods
    inline const mat4 &vpMatrix() const { return vpMatrix_; }
    inline const mat4 &rotateMatrix() const { return rotateMatrix_; }
    inline const mat4 &translateMatrix() const { return translateMatrix_; }
    inline const mat4 &scaleMatrix() const { return scaleMatrix_; }
    inline vec3 getPosition() const { return this->position; }
    inline vec3 getPosition(vec3 offset) const { return this->position + offset; }
    inline vec3 getRotation() const { return this->rotation; }
    inline float getScale() const { return this->scale_; }
    inline uint getRenderPriority() const { return this->renderPriority_; }
    inline vec3 getResolution() const { return this->resolution_; }
    inline string getObjectName() const { return this->objectName; }
    inline ObjectType type() const { return type_; }

    // Misc
    /**
     * @brief Updates translate, rotate and scale matrices. Will modify model attributes if the scene object has an
     * associated parent.
     */
    void updateModelMatrices();
    /**
     * @brief Assigns the SceneObject a parent SceneObject. Will also add this current SceneObject to the parent's list
     * of child objects if parent is not null.
     * @param parent - Pointer to the parent SceneObject to assign to this object.
     */
    void setParent(SceneObject *parent);
    /**
     * @brief Assigns a child object to this SceneObject. Only used for tracking.
     * @param child - Pointer to the child SceneObject to track from this SceneObject.
     */
    void addChild(SceneObject *child);
    /**
     * @brief Removes the given child object from this object's child list.
     * @param child - Pointer to the child object to remove.
     */
    void removeChild(SceneObject *child);

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

    uint renderPriority_ = RENDER_PRIOR_HIGH;

    GfxController *gfxController_;
    SceneObject *parent_ = nullptr;
    std::set<SceneObject *> children_;

    mutex objectLock_;
};
