/**
 * @file SpriteObject.cpp
 * @author Christian Galvez
 * @brief Implementation for SpriteObject
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <SpriteObject.hpp>

SpriteObject::SpriteObject(string spritePath, vec3 position, float scale, unsigned int programId,
        string objectName, ObjectType type, GfxController *gfxController): SceneObject(position,
    vec3(0.0f, 0.0f, 0.0f), objectName, scale, programId, type, gfxController), spritePath_ { spritePath } {
    printf("SpriteObject::SpriteObject: Creating sprite %s\n", objectName.c_str());
    initializeShaderVars();
    initializeSprite();
}

/// @todo Resolution is hardcoded to 720p right now. Add functionality to change this on the fly. Will need to re-send
/// projection matrix.
void SpriteObject::initializeShaderVars() {
    mat4 projection = ortho(0.0f, static_cast<float>(1280), 0.0f, static_cast<float>(720));
    gfxController_->setProgram(programId_);
    auto projectionId = gfxController_->getShaderVariable(programId_, "projection").get();
    gfxController_->sendFloatMatrix(projectionId, 1, glm::value_ptr(projection));
}

void SpriteObject::initializeSprite() {
    cout << "SpriteObject::initializeSprite with path " << spritePath_ << endl;
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

    auto x = this->position.x, y = this->position.y;
    auto x2 = x + (texture->w * scale), y2 = y - (texture->h * scale);
    // Use textures to create each character as an independent object
        gfxController_->initVao(&vao_);
        gfxController_->bindVao(vao_);
        gfxController_->generateBuffer(&vbo_);
        gfxController_->bindBuffer(vbo_);
        // update VBO for each character
        vector<float> vertices = {
            x, y, 0.0f, 0.0f,
            x, y2, 0.0f, 1.0f,
            x2, y2, 1.0f, 1.0f,

            x, y, 0.0f, 0.0f,
            x2, y2, 1.0f, 1.0f,
            x2, y, 1.0f, 0.0f
        };

        cout << "SpriteObject coords: " << endl;
        int formatter = 0;
        for (float pos : vertices) {
            
            cout << pos << ", ";
            if (formatter < 3) {
                formatter++;
            } else {
                formatter = 0;
                cout << endl;
            }
        }
        cout << endl << "DONE" << endl;
        // Send VBO data for each character to the currently bound buffer
        gfxController_->sendBufferData(sizeof(float) * vertices.size(), &vertices[0]);
        gfxController_->enableVertexAttArray(0, 4);
    gfxController_->bindBuffer(0);
    gfxController_->bindVao(0);
}

/// @todo Do something useful here
SpriteObject::~SpriteObject() {
}

void SpriteObject::render() {
    gfxController_->clear(GfxClearMode::DEPTH);
    gfxController_->setProgram(programId_);
    gfxController_->polygonRenderMode(RenderMode::FILL);
    // Find a more clever solution
    gfxController_->bindVao(vao_);
    gfxController_->bindTexture(textureId_);
    gfxController_->drawTriangles(6);
    gfxController_->bindVao(0);
    gfxController_->bindTexture(0);
}

void SpriteObject::update() {
    render();
}

