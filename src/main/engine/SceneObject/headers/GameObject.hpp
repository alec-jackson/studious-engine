/**
 * @file GameObject.hpp
 * @author Christian Galvez
 * @brief GameObject is a SceneObject; can be rendered by a CameraObject
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <ModelImport.hpp>
#include <SceneObject.hpp>
#include <ColliderObject.hpp>

class GameObject: public SceneObject {
 public:
    // Constructurs
    explicit GameObject(Polygon *characterModel, vec3 position, vec3 rotation, float scale,
            string objectName, ObjectType type, GfxController *gfxController);
    explicit GameObject(GfxController *gfxController);
    ~GameObject() override;

    // Setters
    inline void setScale(float scale) { this->scale = scale; }
    inline void setDirectionalLight(vec3 directionalLight) { this->directionalLight = directionalLight; }
    inline void setLuminance(float luminance) { this->luminance = luminance; }
    inline void setProgramId(unsigned int programId) { this->programId_ = programId; }

    // Getters
    inline float getScale() { return this->scale; }
    inline vec3 getDirectionalLight() { return this->directionalLight; }
    inline float getLuminance() { return this->luminance; }
    inline unsigned int getProgramId() { return this->programId_; }

    // Special Getters
    inline Polygon *getModel() { return model; }
    ColliderObject *getCollider();

    // Other methods
    void createCollider(int programId);
    void configureOpenGl();

    void render() override;
    void update() override;

 private:
    Polygon *model;  // Change this to a proper class at some point

    unsigned int vpId, modelId,
        hasTextureId, directionalLightId, luminanceId, rollOffId;

    float luminance;
    float rollOff;

    vector<int> hasTexture;
    vector<unsigned int> vaos_;  // Temporary?
    vec3 directionalLight;

    std::shared_ptr<ColliderObject> collider_;
};
