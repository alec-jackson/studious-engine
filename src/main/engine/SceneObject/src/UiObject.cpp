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

#include <UiObject.hpp>

UiObject::UiObject(string spritePath, vec3 position, float scale, float wScale, float hScale, unsigned int programId,
        string objectName, ObjectType type, GfxController *gfxController): SceneObject(position,
    vec3(0.0f, 0.0f, 0.0f), objectName, scale, programId, type, gfxController), spritePath_ { spritePath }, wScale_ { wScale },
    hScale_ { hScale } {
    printf("UiObject::UiObject: Creating sprite %s\n", objectName.c_str());
    initializeShaderVars();
    initializeSprite();
}

/// @todo Resolution is hardcoded to 720p right now. Add functionality to change this on the fly. Will need to re-send
/// projection matrix.
void UiObject::initializeShaderVars() {
    mat4 projection = ortho(0.0f, static_cast<float>(1280), 0.0f, static_cast<float>(720));
    gfxController_->setProgram(programId_);
    auto projectionId = gfxController_->getShaderVariable(programId_, "projection").get();
    gfxController_->sendFloatMatrix(projectionId, 1, glm::value_ptr(projection));
    wScaleId_ = gfxController_->getShaderVariable(programId_, "wScale").get();
    hScaleId_ = gfxController_->getShaderVariable(programId_, "hScale").get();
    modelMatId_ = gfxController_->getShaderVariable(programId_, "model").get();
    gfxController_->sendFloat(wScaleId_, wScale_);
    gfxController_->sendFloat(hScaleId_, hScale_);
    gfxController_->sendFloatMatrix(modelMatId_, 1, glm::value_ptr(modelMat_));
}

void UiObject::generateVertexBase(float *vertexData, int triIdx, float x, float y, float x2, float y2) {
    auto dT = (1.0f/3.0f);
    auto tX = (triIdx % 3) * dT; // 0.33 target...
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
    cout << "UiObject::generateVertex: " << endl;
    for (int i = 0; i < vertices.size(); ++i) {
        printf("%f, ", vertices[i]);
        if (i % 4 == 3) {
            cout << endl;
        }
    }
    // Convert triangle index to vertexData offset
    // 2 triangles, 3 vertices per tri, 4 points per vertex
    auto offset = triIdx * 2 * 3 * 4;
    auto index = 0;
    // Insert vertex data into vertexData buffer (lazy, can fix this later)
    for (auto v : vertices) {
        vertexData[offset + index++] = v;
    }
}

float *UiObject::generateVertices(float x, float y, float iFx, float iFy) {
    // Allocate memory for vertex data
    float *vertexData = new float[24 * 9];

    // Create the triangles from top left to right
    for (int i = 1; i < 4; ++i) {
        auto y2 = y - (iFy * i);
        auto y1 = y - (iFy * (i-1));
        for (int j = 1; j < 4; ++j) {
            auto x2 = x + (iFx * j);
            auto x1 = x + (iFx * (j - 1));
            auto idx = (j - 1) + (i - 1) * 3;
            printf("UiObject::generateVertices: Generating vertexData for idx %d\n", idx);
            generateVertexBase(vertexData, idx, x1, y1, x2, y2);
        }
    }

    return vertexData;
}

void UiObject::initializeSprite() {
    cout << "UiObject::initializeSprite with path " << spritePath_ << endl;
    SDL_Surface *texture = IMG_Load(spritePath_.c_str());
    auto textureFormat = texture->format->Amask ? TexFormat::RGBA : TexFormat::RGB;
    // Send texture image to OpenGL
    gfxController_->generateTexture(&textureId_);
    gfxController_->bindTexture(textureId_);
    gfxController_->sendTextureData(texture->w, texture->h, textureFormat, texture->pixels);
    gfxController_->setTexParam(TexParam::WRAP_MODE_S, TexVal(TexValType::CLAMP_TO_EDGE));
    gfxController_->setTexParam(TexParam::WRAP_MODE_T, TexVal(TexValType::CLAMP_TO_EDGE));
    gfxController_->setTexParam(TexParam::MAGNIFICATION_FILTER, TexVal(TexValType::NEAREST_NEIGHBOR));
    gfxController_->setTexParam(TexParam::MINIFICATION_FILTER, TexVal(TexValType::NEAREST_MIPMAP));
    gfxController_->setTexParam(TexParam::MIPMAP_LEVEL, TexVal(10));
    gfxController_->generateMipMap();

    auto x = 0.0f, y = 0.0f;
    auto incrementFactorX = (texture->w * scale / 3.0f);
    auto incrementFactorY = (texture->h * scale / 3.0f);
    // Use textures to create each character as an independent object
        gfxController_->initVao(&vao_);
        gfxController_->bindVao(vao_);
        gfxController_->generateBuffer(&vbo_);
        gfxController_->bindBuffer(vbo_);

        vertexData_ = generateVertices(x, y, incrementFactorX, incrementFactorY);
        // Send VBO data for each character to the currently bound buffer
        gfxController_->sendBufferData(sizeof(float) * 24 * 9, vertexData_);
        gfxController_->enableVertexAttArray(0, 4);
    gfxController_->bindBuffer(0);
    gfxController_->bindVao(0);
}

/// @todo Do something useful here
UiObject::~UiObject() {
}

void UiObject::render() {
    // Update model matrices
    translateMatrix_ = glm::translate(mat4(1.0f), position);
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

void UiObject::setWScale(float scale) {
    wScale_ = scale;
}

void UiObject::setHScale(float scale) {
    hScale_ = scale;
}

float UiObject::getHScale() {
    return hScale_;
}

float UiObject::getWScale() {
    return wScale_;
}
