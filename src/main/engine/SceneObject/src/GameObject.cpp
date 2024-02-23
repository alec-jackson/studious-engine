/**
 * @file GameObject.cpp
 * @author Christian Galvez
 * @brief Implementation for GameObject
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <GameObject.hpp>

/**
 * @brief GameObject constructor
 * 
 * @param characterModel Underlying Polygon object for rendering this GameObject
 * @param position Starting position of the GameObject
 * @param rotation Starting rotation of the GameObject
 * @param scale Starting scale of the GameObject
 * @param camera Camera used to render the GameObject
 * @param objectName Friendly name for the object used to identify it in the scene
 * @param gfxController Graphics controller for rendering the game scene
 */
GameObject::GameObject(Polygon *characterModel, vec3 position, vec3 rotation, GLfloat scale,
    string objectName, ObjectType type, GfxController *gfxController):
    SceneObject(position, rotation, objectName, scale, characterModel->programId, type, gfxController),
    model { characterModel } {
    luminance = 1.0f;
    rollOff = 0.9f;  // Rolloff describes the intensity of the light dropoff
    directionalLight = vec3(0, 0, 0);
    // Populate the hasTexture vector with texture info
    for (int i = 0; i < model->numberOfObjects; i++) {
        if (model->textureCoordsId[i] == UINT_MAX) {
            hasTexture.push_back(0);  // No texture found for obj i
        } else {
            hasTexture.push_back(1);  // Texture found for obj i
        }
    }
    scaleMatrix_ = glm::scale(vec3(scale, scale, scale));
    translateMatrix_ = glm::translate(mat4(1.0f), position);
    rotateMatrix_ = glm::rotate(mat4(1.0f), glm::radians(rotation[0]),
            vec3(1, 0, 0))  *glm::rotate(mat4(1.0f), glm::radians(rotation[1]),
            vec3(0, 1, 0))  *glm::rotate(mat4(1.0f), glm::radians(rotation[2]),
            vec3(0, 0, 1));
    // Grab IDs for shared variables between app and program (shader)
    modelId = gfxController_->getShaderVariable(programId, "model").get();
    vpId = gfxController_->getShaderVariable(programId, "VP").get();
    hasTextureId = gfxController_->getShaderVariable(programId, "hasTexture").get();
    directionalLightId = gfxController_->getShaderVariable(programId, "directionalLight").get();
    luminanceId = gfxController_->getShaderVariable(programId, "luminance").get();
    rollOffId = gfxController_->getShaderVariable(programId, "rollOff").get();
    vpMatrix_ = mat4(1.0f);  // Default VP matrix to identity matrix
}

/**
 * @brief GameObject destructor
 */
GameObject::~GameObject() {
    /// @todo: Run cleanup methods here
    cout << "Destroying gameobject" << objectName << endl;
    // Delete collider
    if (collider_ != nullptr) delete collider_;
    deleteTextures();
}

/**
 * @brief Updates and returns the GameObject's collider
 * 
 * @return ColliderObject* for the GameObject
 */
ColliderObject *GameObject::getCollider(void) {
    // Update collider before returning it
    collider_->updateCollider();
    return collider_;
}

/**
 * @brief Creates a collider for this game object
 * 
 * @param programId Program used to render the collider (collider shaders)
 */
void GameObject::createCollider(int programId) {
    printf("GameObject::createCollider: Creating collider for object %s\n", objectName.c_str());
    collider_ = new ColliderObject(this->getModel(), programId, translateMatrix_, scaleMatrix_, vpMatrix_,
        ObjectType::GAME_OBJECT, gfxController_);
}

void GameObject::update() {
    // Update the gfx controller before rendering
    //gfxController_->update();
    render();
}

/**
 * @brief Steps through all objects in the model and renders them one at a time. A single Polygon can have several
 * models connected to it, which can have their own textures, etc.
 */
void GameObject::render() {
    // Send GameObject to render method
    // Draw each shape individually
    for (int i = 0; i < model->numberOfObjects; i++) {
        gfxController_->setProgram(programId);
        gfxController_->polygonRenderMode(RenderMode::FILL);
        // Update our model transformation matrices
        translateMatrix_ = glm::translate(mat4(1.0f), position);
        rotateMatrix_ = glm::rotate(mat4(1.0f), glm::radians(rotation[0]),
                vec3(1, 0, 0))  *glm::rotate(mat4(1.0f), glm::radians(rotation[1]),
                vec3(0, 1, 0))  *glm::rotate(mat4(1.0f), glm::radians(rotation[2]),
                vec3(0, 0, 1));

        scaleMatrix_ = glm::scale(vec3(scale, scale, scale));
        auto modelMatrix = translateMatrix_ * rotateMatrix_ * scaleMatrix_;
        // Send our shared variables over to our program (shader)
        gfxController_->sendFloat(luminanceId, luminance);
        gfxController_->sendFloat(rollOffId, rollOff);
        gfxController_->sendFloatVector(directionalLightId, 1, glm::value_ptr(directionalLight));
        gfxController_->sendFloatMatrix(vpId, 1, glm::value_ptr(vpMatrix_));
        gfxController_->sendFloatMatrix(modelId, 1, glm::value_ptr(modelMatrix));
        gfxController_->sendInteger(hasTextureId, hasTexture[i]);
        if (hasTexture[i]) {
            // Bind texture to sampler for polygon rendering below
            gfxController_->bindTexture(model->textureId[i], model->textureUniformId);
        }
        // Actually start drawing polygons :)
        gfxController_->render(model->shapeBufferId[i], model->textureCoordsId[i], model->normalBufferId[i],
        model->pointCount[i] * 3);
    }
    if (collider_ != nullptr) collider_->render();
    }

void GameObject::deleteTextures() {
    cout << "GameObject::deleteTextures" << endl;
    for (int i = 0; i < model->numberOfObjects; i++) {
        if (hasTexture[i]) {
            glDeleteTextures(1, &textureId);
            hasTexture[i] = false;
        }
    }
    auto error = glGetError();
    if (error != 0) {
        fprintf(stderr, "GameObject::deleteTextures: Error %d\n", error);
    }
}
