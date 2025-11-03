/**
 * @file TextObject.cpp
 * @author Christian Galvez
 * @brief Implementation of TextObject
 * @version 0.1
 * @date 2023-07-28
 *
 * @copyright Copyright (c) 2023
 *
 */
#include <TextObject.hpp>
#include <cstdio>
#include <vector>
#include <string>

TextObject::TextObject(string message, vec3 position, float scale, string fontPath, float charSpacing, int charPoint,
    uint programId, string objectName, ObjectType type, GfxController *gfxController): SceneObject(position,
    vec3(0.0f, 0.0f, 0.0f), scale, programId, type, objectName, gfxController), charPadding_ { charSpacing },
    message_  { message }, fontPath_ { fontPath }, charPoint_ { charPoint }, cutoff_ { vec3(0.0f, 9000.0f, 0.0f) },
    textColor_ { vec4(1.0f) } {
    printf("TextObject::TextObject: Creating message %s\n", message.c_str());
    initializeShaderVars();
    initializeText();
    createMessage();
}

/// @todo Resolution is hardcoded to 720p right now. Add functionality to change this on the fly. Will need to re-send
/// projection matrix.
void TextObject::initializeShaderVars() {
    mat4 projection = ortho(0.0f, 1280.0f, 0.0f, 720.0f);
    gfxController_->setProgram(programId_);
    projectionId_ = gfxController_->getShaderVariable(programId_, "projection").get();
    gfxController_->sendFloatMatrix(projectionId_, 1, glm::value_ptr(projection));
    modelMatId_ = gfxController_->getShaderVariable(programId_, "model").get();
    gfxController_->sendFloatMatrix(modelMatId_, 1, glm::value_ptr(modelMat_));
    cutoffId_ = gfxController_->getShaderVariable(programId_, "cutoff").get();
    gfxController_->sendFloatVector(cutoffId_, 1, VectorType::GFX_3D, glm::value_ptr(cutoff_));
}

void TextObject::initializeText() {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        fprintf(stderr, "TextObject::initializeText: Could not init FreeType Library\n");
        throw std::runtime_error("Failed to initialize FreeType Library");
    }
    FT_Face face;
    if (FT_New_Face(ft, fontPath_.c_str(), 0, &face)) {
        fprintf(stderr, "TextObject::initializeText: FREETYPE: Failed to load font\n");
        throw std::runtime_error("Failed to load font");
    } else {
        FT_Set_Pixel_Sizes(face, 0, charPoint_);
        for (unsigned char c = 0; c < 128; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                fprintf(stderr, "TextObject::initializeText: FREETYPE: Failed to load glyph\n");
                continue;
            }
            unsigned int textureId = 0;
            // Generate OpenGL textures for Freetype font rasterizations
            gfxController_->generateTexture(&textureId);
            gfxController_->bindTexture(textureId, GfxTextureType::NORMAL);
            gfxController_->sendTextureData(
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                TexFormat::BITMAP,
                face->glyph->bitmap.buffer);
            gfxController_->setTexParam(TexParam::WRAP_MODE_S, TexVal(TexValType::CLAMP_TO_EDGE),
                GfxTextureType::NORMAL);
            gfxController_->setTexParam(TexParam::WRAP_MODE_T, TexVal(TexValType::CLAMP_TO_EDGE),
                GfxTextureType::NORMAL);
            gfxController_->setTexParam(TexParam::MINIFICATION_FILTER, TexVal(TexValType::GFX_LINEAR),
                GfxTextureType::NORMAL);
            gfxController_->setTexParam(TexParam::MAGNIFICATION_FILTER, TexVal(TexValType::GFX_LINEAR),
                GfxTextureType::NORMAL);

            Character character = {
                textureId,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            characters.insert(std::pair<char, Character>(c, character));
        }
        gfxController_->bindTexture(0, GfxTextureType::NORMAL);
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void TextObject::createMessage() {
    auto x = 0, y = 0;
    auto spacing = 1.0f;  /// @todo Make this adjustable
    // Use textures to create each character as an independent object
    for (auto character : message_) {
        Character ch = characters[character];
        float xpos = x + ch.Bearing.x * scale_;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale_;
        float w = ch.Size.x * scale_;
        float h = ch.Size.y * scale_;
        if (character == '\n') {
            x = 0;
            y -= (h * (spacing + 1.0f));
            continue;
        }
        unsigned int vao;
        gfxController_->initVao(&vao);
        gfxController_->bindVao(vao);
        unsigned int vbo;
        gfxController_->generateBuffer(&vbo);
        gfxController_->bindBuffer(vbo);
        // update VBO for each character
        vector<float> vertices = {
            xpos,     ypos + h,   0.0f, 0.0f,
            xpos,     ypos,       0.0f, 1.0f,
            xpos + w, ypos,       1.0f, 1.0f,

            xpos,     ypos + h,   0.0f, 0.0f,
            xpos + w, ypos,       1.0f, 1.0f,
            xpos + w, ypos + h,   1.0f, 0.0f
        };

        // Send VBO data for each character to the currently bound buffer
        gfxController_->sendBufferData(sizeof(float) * vertices.size(), &vertices[0]);
        gfxController_->enableVertexAttArray(0, 4, sizeof(float), 0);
        vaos_.push_back(vao);
        // Update x/y
        x = w == 0 ? x + static_cast<float>(ch.Advance / 100.0f) : xpos + w + charPadding_;
    }
    gfxController_->bindBuffer(0);
    gfxController_->bindVao(0);
}

/// @todo Do something useful here
TextObject::~TextObject() {
}

void TextObject::render() {
    VISIBILITY_CHECK;
    // Update model matrices
    updateModelMatrices();
    modelMat_ = translateMatrix_;
    gfxController_->clear(GfxClearMode::DEPTH);
    gfxController_->setProgram(programId_);
    gfxController_->polygonRenderMode(RenderMode::FILL);
    gfxController_->sendFloatMatrix(modelMatId_, 1, glm::value_ptr(modelMat_));
    gfxController_->sendFloatVector(cutoffId_, 1, VectorType::GFX_3D, glm::value_ptr(cutoff_));
    /// @todo optimize this...
    auto textColorId = gfxController_->getShaderVariable(programId_, "textColor").get();
    gfxController_->sendFloatVector(textColorId, 1, VectorType::GFX_4D, glm::value_ptr(textColor_));
    // Find a more clever solution
    auto index = 0;
    for (auto character : message_) {
        if (character == '\n') continue;
        gfxController_->bindVao(vaos_[index++]);
        Character ch = characters[character];
        gfxController_->bindTexture(ch.TextureID, GfxTextureType::NORMAL);
        gfxController_->drawTriangles(6);
    }
    gfxController_->bindVao(0);
    gfxController_->bindTexture(0, GfxTextureType::NORMAL);
}

void TextObject::update() {
    render();
}

/**
 * @brief Update the text object with a new message. If the incoming message is the same as the existing message, then
 * the message is not updated.
 *
 * @param message Incoming message to set TextObject to.
 */
void TextObject::setMessage(string message) {
    if (!message.compare(message_)) return;
    // Perform cleanup on existing VAOs
    gfxController_->bindVao(0);
    for (auto vao : vaos_) {
        gfxController_->deleteVao(&vao);
    }
    for (auto vbo : vbos_) {
        gfxController_->deleteBuffer(&vbo);
    }
    vaos_.clear();
    vaos_.shrink_to_fit();
    vbos_.clear();
    vbos_.shrink_to_fit();
    message_ = message;
    createMessage();
}
