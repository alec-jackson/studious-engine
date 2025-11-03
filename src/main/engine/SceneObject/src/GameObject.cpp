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
#include <string>
#include <cstdio>
#include <iostream>
#include <memory>

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
GameObject::GameObject(std::shared_ptr<Polygon> characterModel, vec3 position, vec3 rotation, float scale,
    uint programId, string objectName, ObjectType type, GfxController *gfxController):
    SceneObject(position, rotation, scale, programId, type, objectName, gfxController),
    model_ { characterModel } {
    // Enforce the model is VALID!
    assert(model_.get() != nullptr);
    configureOpenGl();
    luminance = 1.0f;
    rollOff = 0.9f;  // Rolloff describes the intensity of the light dropoff
    directionalLight = vec3(0, 0, 0);
    scaleMatrix_ = glm::scale(vec3(scale_, scale_, scale_));
    translateMatrix_ = glm::translate(mat4(1.0f), position);
    rotateMatrix_ = glm::rotate(mat4(1.0f), glm::radians(rotation[0]),
            vec3(1, 0, 0))  *glm::rotate(mat4(1.0f), glm::radians(rotation[1]),
            vec3(0, 1, 0))  *glm::rotate(mat4(1.0f), glm::radians(rotation[2]),
            vec3(0, 0, 1));
    // Grab IDs for shared variables between app and program (shader)
    modelId = gfxController_->getShaderVariable(programId_, "model").get();
    vpId = gfxController_->getShaderVariable(programId_, "VP").get();
    hasTextureId = gfxController_->getShaderVariable(programId_, "hasTexture").get();
    directionalLightId = gfxController_->getShaderVariable(programId_, "directionalLight").get();
    luminanceId = gfxController_->getShaderVariable(programId_, "luminance").get();
    rollOffId = gfxController_->getShaderVariable(programId_, "rollOff").get();
    vpMatrix_ = mat4(1.0f);  // Default VP matrix to identity matrix
}

// What is this used for?
GameObject::GameObject(GfxController *gfxController) :
    SceneObject(ObjectType::GAME_OBJECT, "EmptyModel", gfxController) {
}

/**
 * @brief Configures the created object with OpenGL. This step is required for object rendering.
 *
 * @param polygon to configure OpenGL context for.
 * @param objectId index of the object to configure OpenGL for relative to other objects in the parsed .obj file.
 */
void GameObject::configureOpenGl() {
    printf("GameObject::configureOpenGl: Configuring for %s with %zu objects\n", objectName_.c_str(),
        model_.get()->modelMap.size());
    for (auto &modelPair : model_->modelMap) {
        gfxController_->initVao(&modelPair.second.get()->vao);
        gfxController_->bindVao(modelPair.second.get()->vao);
        // Generate vertex buffer
        gfxController_->generateBuffer(&modelPair.second.get()->shapeBufferId);
        gfxController_->bindBuffer(modelPair.second.get()->shapeBufferId);
        gfxController_->sendBufferData(sizeof(float) * modelPair.second.get()->pointCount * 9,
            modelPair.second.get()->vertices.data());
        gfxController_->enableVertexAttArray(0, 3, sizeof(float), 0);
        // Generate normal buffer
        gfxController_->generateBuffer(&modelPair.second.get()->normalBufferId);
        gfxController_->bindBuffer(modelPair.second.get()->normalBufferId);
        gfxController_->sendBufferData(sizeof(float) * modelPair.second.get()->pointCount * 9,
            modelPair.second.get()->normalCoords.data());
        gfxController_->enableVertexAttArray(2, 3, sizeof(float), 0);
        // Specific case where the current object does not get a texture
        auto materialName = modelPair.second.get()->materialName;
        auto mmit = model_.get()->materialMap.find(materialName);
        if (model_.get()->materialMap.end() == mmit) {
            fprintf(stderr, "Material not found, cannot join.\n");
        }
        for (auto ent : model_.get()->materialMap) {
            fprintf(stderr, "materialMap is %s -> %s\n",
                ent.first.c_str(), ent.second.get()->name.c_str());
        }
        if (model_.get()->materialMap.end() == mmit ||
            mmit->second->pathToTextureFile.empty()) {
            fprintf(stderr,
                "GameObject::configureOpenGl: Either no material for %s "
                "or material has no texture defined! matname[%s]\n",
                modelPair.first.c_str(), materialName.c_str());
            gfxController_->bindVao(0);
            continue;
        }
        SDL_Surface *texture = IMG_Load(mmit->second->pathToTextureFile.c_str());
        cout << "Loading texture: " << mmit->second->pathToTextureFile << "\n";
        if (texture == NULL) {
            cerr << "Failed to create SDL_Surface texture!\n";
            gfxController_->bindVao(0);
            continue;
        }

        auto textureFormat = texture->format->Amask ? TexFormat::RGBA : TexFormat::RGB;
        // Send texture image to OpenGL
        gfxController_->generateTexture(&modelPair.second.get()->textureId);
        gfxController_->bindTexture(modelPair.second.get()->textureId, GfxTextureType::NORMAL);
        gfxController_->sendTextureData(texture->w, texture->h, textureFormat, texture->pixels);
        gfxController_->setTexParam(TexParam::WRAP_MODE_S, TexVal(TexValType::CLAMP_TO_EDGE), GfxTextureType::NORMAL);
        gfxController_->setTexParam(TexParam::WRAP_MODE_T, TexVal(TexValType::CLAMP_TO_EDGE), GfxTextureType::NORMAL);
        gfxController_->setTexParam(TexParam::MAGNIFICATION_FILTER, TexVal(TexValType::NEAREST_NEIGHBOR),
            GfxTextureType::NORMAL);
        gfxController_->setTexParam(TexParam::MINIFICATION_FILTER, TexVal(TexValType::NEAREST_MIPMAP),
            GfxTextureType::NORMAL);
        gfxController_->setTexParam(TexParam::MIPMAP_LEVEL, TexVal(10), GfxTextureType::NORMAL);
        gfxController_->generateMipMap();

        // Send texture coords to OpenGL
        gfxController_->generateBuffer(&modelPair.second.get()->textureCoordsId);
        gfxController_->bindBuffer(modelPair.second.get()->textureCoordsId);
        gfxController_->sendBufferData(sizeof(float) * modelPair.second.get()->pointCount * 6,
            modelPair.second.get()->textureCoords.data());
        gfxController_->enableVertexAttArray(1, 2, sizeof(float), 0);

        SDL_FreeSurface(texture);
        gfxController_->bindVao(0);
        gfxController_->bindTexture(0, GfxTextureType::NORMAL);
    }
    model_->textureUniformId = gfxController_->getShaderVariable(programId_, "mytexture").get();
}

/**
 * @brief GameObject destructor
 */
GameObject::~GameObject() {
}

/**
 * @brief Creates a collider for this game object
 *
 * @param programId Program used to render the collider (collider shaders)
 */
void GameObject::createCollider() {
    printf("GameObject::createCollider: Creating collider for object %s\n", objectName_.c_str());
    auto colliderProg = gfxController_->getProgramId(COLLIDEROBJECT_PROG_NAME);
    if (!colliderProg.isOk()) {
        fprintf(stderr,
            "GameObject::createCollider: Failed to create collider! '%s' shader not defined!\n",
            COLLIDEROBJECT_PROG_NAME);
        return;
    }
    collider_ = std::make_shared<ColliderObject>(this->getModel(), colliderProg.get(), this);
}

void GameObject::update() {
    render();
}

/**
 * @brief Steps through all objects in the model and renders them one at a time. A single Polygon can have several
 * models connected to it, which can have their own textures, etc.
 */
void GameObject::render() {
    VISIBILITY_CHECK;
    if (model_.get() == nullptr) return;
    // Send GameObject to render method
    // Draw each shape individually
    for (auto &modelPair : model_.get()->modelMap) {
        int hasTexture = modelPair.second.get()->textureCoordsId != UINT_MAX ? 1 : 0;
        gfxController_->setProgram(programId_);
        gfxController_->polygonRenderMode(RenderMode::FILL);
        // Update our model transformation matrices
        updateModelMatrices();
        auto modelMatrix = translateMatrix_ * rotateMatrix_ * scaleMatrix_;
        // Send our shared variables over to our program (shader)
        gfxController_->sendFloat(luminanceId, luminance);
        gfxController_->sendFloat(rollOffId, rollOff);
        gfxController_->sendFloatVector(directionalLightId, 1, VectorType::GFX_3D, glm::value_ptr(directionalLight));
        gfxController_->sendFloatMatrix(vpId, 1, glm::value_ptr(vpMatrix_));
        gfxController_->sendFloatMatrix(modelId, 1, glm::value_ptr(modelMatrix));
        gfxController_->sendInteger(hasTextureId, hasTexture);
        gfxController_->bindVao(modelPair.second.get()->vao);
        if (hasTexture) {
            // textureUniformId points to the sampler2D in GLSL, point it to texture unit 0
            gfxController_->sendInteger(model_.get()->textureUniformId, 0);
            // Bind texture to sampler for polygon rendering below
            gfxController_->bindTexture(modelPair.second.get()->textureId, GfxTextureType::NORMAL);
        }
        gfxController_->drawTriangles(modelPair.second.get()->pointCount * 3);
        gfxController_->bindVao(0);
    }
    if (collider_.use_count() > 0) collider_.get()->update();
    }
