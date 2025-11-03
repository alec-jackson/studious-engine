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
#include <ColliderExt.hpp>
#include <winsup.hpp>

class GameObject: public SceneObject, public ColliderExt {
 public:
    // Constructurs
    explicit GameObject(std::shared_ptr<Polygon> characterModel, vec3 position, vec3 rotation, float scale,
            uint programId, string objectName, ObjectType type, GfxController *gfxController);
    explicit GameObject(GfxController *gfxController);
    ~GameObject();

    // Setters
    inline void setDirectionalLight(vec3 directionalLight) { this->directionalLight = directionalLight; }
    inline void setLuminance(float luminance) { this->luminance = luminance; }
    inline void setProgramId(unsigned int programId) { this->programId_ = programId; }

    // Getters
    inline vec3 getDirectionalLight() { return this->directionalLight; }
    inline float getLuminance() { return this->luminance; }
    inline unsigned int getProgramId() { return this->programId_; }

    // Special Getters
    inline std::shared_ptr<Polygon> getModel() { return model_; }

    // Other methods
    void createCollider() override;
    void configureOpenGl();

    void render() override;
    void update() override;

 private:
    std::shared_ptr<Polygon> model_;

    unsigned int vpId, modelId,
        hasTextureId, directionalLightId, luminanceId, rollOffId;

    float luminance;
    float rollOff;

    vector<int> hasTexture;
    vec3 directionalLight;
};
