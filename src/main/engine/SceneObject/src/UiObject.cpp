/**
 * @file UiObject.cpp
 * @author Christian Galvez
 * @brief Implementation for UiObject
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <string>
#include <vector>
#include <cstdio>
#include <iostream>
#include <memory>
#include <UiObject.hpp>

UiObject::UiObject(string spritePath, vec3 position, float scale, float wScale, float hScale, unsigned int programId,
        string objectName, ObjectType type, ObjectAnchor anchor, GfxController *gfxController): GameObject2D(
            spritePath, position, scale, programId, objectName, type, anchor, gfxController),
        wScale_ { wScale }, hScale_ { hScale } {
    printf("UiObject::UiObject: Creating sprite %s\n", objectName.c_str());
    GameObject2D::initializeTextureData();
    initializeVertexData();
    initializeShaderVars();
}

/// @todo Resolution is hardcoded to 720p right now. Add functionality to change this on the fly. Will need to re-send
/// projection matrix.
void UiObject::initializeShaderVars() {
    GameObject2D::initializeShaderVars();
    wScaleId_ = gfxController_->getShaderVariable(programId_, "wScale").get();
    hScaleId_ = gfxController_->getShaderVariable(programId_, "hScale").get();
    vertexIndexId_ = gfxController_->getShaderVariable(programId_, "vertexIndex").get();
    gfxController_->sendFloat(wScaleId_, wScale_);
    gfxController_->sendFloat(hScaleId_, hScale_);
    gfxController_->sendFloatMatrix(modelMatId_, 1, glm::value_ptr(modelMat_));
}

void UiObject::generateVertexBase
    (std::shared_ptr<float[]> vertexData, int triIdx, float x, float y, float x2, float y2) {
    auto dT = (1.0f/3.0f);
    auto tX = (triIdx % 3) * dT;  // 0.33 target...
    auto tY = (triIdx / 3) * dT;
    auto tX2 = tX + dT;
    auto tY2 = tY + dT;
    vector<float> vertices = {
            x, y, tX, tY,
            x, y2, tX, tY2,
            x2, y2, tX2, tY2,

            x, y, tX, tY,
            x2, y2, tX2, tY2,
            x2, y, tX2, tY
    };
    // Convert triangle index to vertexData offset
    // 2 triangles, 3 vertices per tri, 4 points per vertex
    auto offset = triIdx * 2 * 3 * 4;
    auto index = 0;
    // Insert vertex data into vertexData buffer (lazy, can fix this later)
    for (auto v : vertices) {
        vertexData[offset + index++] = v;
    }
}

std::shared_ptr<float[]> UiObject::generateVertices(float x, float y, float iFx, float iFy) {
    // Allocate memory for vertex data
    std::shared_ptr<float[]> vertexData(new float[24 * 9], std::default_delete<float[]>());

    // Create the triangles from top left to right
    for (int i = 1; i < 4; ++i) {
        auto y2 = y - (iFy * i);
        auto y1 = y - (iFy * (i-1));
        for (int j = 1; j < 4; ++j) {
            auto x2 = x + (iFx * j);
            auto x1 = x + (iFx * (j - 1));
            auto idx = (j - 1) + (i - 1) * 3;
            generateVertexBase(vertexData, idx, x1, y1, x2, y2);
        }
    }
    return vertexData;
}

void UiObject::initializeVertexData() {
    // Perform anchor points here
    auto x = 0.0f, y = 0.0f;
    switch (anchor_) {
        case BOTTOM_LEFT:
            x = 0.0f;
            y = 0.0f;
            break;
        case CENTER:
            x = -1 * ((textureWidth_) / 2.0f);
            y = (textureHeight_) / 2.0f;
            break;
        default:
            fprintf(stderr, "UiObject::initializeVertexData: Unsupported anchor type %d\n", anchor_);
            assert(false);
            break;
    }
    auto incrementFactorX = (textureWidth_ * scale / 3.0f);
    auto incrementFactorY = (textureHeight_ * scale / 3.0f);
    // Use textures to create each character as an independent object
    gfxController_->initVao(&vao_);
    gfxController_->bindVao(vao_);
    gfxController_->generateBuffer(&vbo_);
    gfxController_->bindBuffer(vbo_);

    vertexData_ = generateVertices(x, y, incrementFactorX, incrementFactorY);
    gfxController_->sendBufferData(sizeof(float) * 24 * 9, vertexData_.get());
    gfxController_->enableVertexAttArray(0, 4);

    // Generate the vertex index buffer and send it
    vertexIndexData_ = std::shared_ptr<float[]>(new float[24 * 3], std::default_delete<float[]>());
    for (int i = 0; i < 24 * 3; ++i) {
        vertexIndexData_[i] = i;
    }

    gfxController_->generateBuffer(&vertexIndexVbo_);
    gfxController_->bindBuffer(vertexIndexVbo_);
    gfxController_->sendBufferData(sizeof(float) * 24 * 9, vertexIndexData_.get());
    gfxController_->enableVertexAttArray(1, 1);
    gfxController_->bindBuffer(0);
    gfxController_->bindVao(0);
}

UiObject::~UiObject() {
}

void UiObject::render() {
    // Update model matrices
    translateMatrix_ = glm::translate(mat4(1.0f), position);
    rotateMatrix_ = glm::rotate(mat4(1.0f), glm::radians(rotation[0]),
            vec3(1, 0, 0))  *glm::rotate(mat4(1.0f), glm::radians(rotation[1]),
            vec3(0, 1, 0))  *glm::rotate(mat4(1.0f), glm::radians(rotation[2]),
            vec3(0, 0, 1));
    // scaleMatrix_ = glm::scale(vec3(scale, scale, scale));
    // modelMat_ = translateMatrix_ * rotateMatrix_ * scaleMatrix_;
    // We wont do scale normally for now due to a bug
    modelMat_ = translateMatrix_;
    gfxController_->clear(GfxClearMode::DEPTH);
    gfxController_->setProgram(programId_);
    gfxController_->polygonRenderMode(RenderMode::FILL);
    gfxController_->sendFloat(wScaleId_, wScale_);
    gfxController_->sendFloat(hScaleId_, hScale_);
    gfxController_->sendFloatMatrix(modelMatId_, 1, glm::value_ptr(modelMat_));
    // Find a more clever solution
    gfxController_->bindVao(vao_);
    gfxController_->bindTexture(textureId_);
    gfxController_->drawTriangles(6 * 9);
    gfxController_->bindVao(0);
    gfxController_->bindTexture(0);
}

void UiObject::update() {
    render();
}

void UiObject::setWStretch(float scale) {
    wScale_ = scale;
}

void UiObject::setHStretch(float scale) {
    hScale_ = scale;
}

vec3 UiObject::getStretch() {
    return vec3(wScale_, hScale_, 0.0f);
}

