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

/// @todo: Finish this code - lots of hard-coded values
TextObject::TextObject(string message, string fontPath, GLuint programId, string objectName, ObjectType type,
              GfxController *gfxController): SceneObject(vec3(300.0f, 300.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f),
    objectName, 1.0f, programId, type, gfxController), message_  { message }, fontPath_ { fontPath } {
    mat4 projection = ortho(0.0f, static_cast<float>(1280), 0.0f, static_cast<float>(720));
    cout << "Initializing text with message " << message << endl;

    gfxController_->setProgram(programId);
    auto projectionId = gfxController_->getShaderVariable(programId, "projection").get();
    gfxController_->sendFloatMatrix(projectionId, 1, glm::value_ptr(projection));
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        throw std::runtime_error("Failed to initialize FreeType Library");
    }
    FT_Face face;
    if (FT_New_Face(ft, fontPath_.c_str(), 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        throw std::runtime_error("Failed to load font");
    } else {
        FT_Set_Pixel_Sizes(face, 0, 48);
        for (unsigned char c = 0; c < 128; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                cout << "ERROR::FREETYTPE: Failed to load Glyph\n";
                continue;
            }
            GLuint textureId;
            gfxController_->generateTexture(&textureId);
            gfxController_->bindTexture(textureId);
            gfxController_->sendTextureData(
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                TexFormat::BITMAP,
                face->glyph->bitmap.buffer);

            gfxController_->setTexParam(TexParam::WRAP_MODE_S, TexVal(TexValType::CLAMP_TO_EDGE));
            gfxController_->setTexParam(TexParam::WRAP_MODE_T, TexVal(TexValType::CLAMP_TO_EDGE));
            gfxController_->setTexParam(TexParam::MINIFICATION_FILTER, TexVal(TexValType::GFX_LINEAR));
            gfxController_->setTexParam(TexParam::MAGNIFICATION_FILTER, TexVal(TexValType::GFX_LINEAR));

            Character character = {
                textureId,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            characters.insert(std::pair<char, Character>(c, character));
        }
        gfxController_->bindTexture(0);
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    gfxController_->initVao(&vao_);
    gfxController_->bindVao(vao_);
    gfxController_->generateBuffer(&VBO);
    gfxController_->bindBuffer(VBO);
    gfxController_->sendBufferData(sizeof(GLfloat) * 6 * 4, nullptr);
    gfxController_->enableVertexAttArray(0, 4);
    gfxController_->bindBuffer(0);
    gfxController_->bindVao(0);
}

/// @todo Do something useful here
TextObject::~TextObject() {
}

void TextObject::render() {
    gfxController_->clear(GfxClearMode::DEPTH);
    vec3 color = vec3(1.0f);
    int x = this->position.x, y = this->position.y;
    gfxController_->setProgram(programId);
    gfxController_->polygonRenderMode(RenderMode::FILL);
    auto textColorId = gfxController_->getShaderVariable(programId, "textColor").get();
    gfxController_->sendFloatVector(textColorId, 1, &color[0]);
    gfxController_->bindVao(vao_);
    for (auto c = message_.begin(); c != message_.end(); c++) {
        Character ch = characters[*c];
        GLfloat xpos = x + ch.Bearing.x * scale;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;
        // update VBO for each character
        vector<GLfloat> vertices = {
            xpos,     ypos + h,   0.0f, 0.0f,
            xpos,     ypos,       0.0f, 1.0f,
            xpos + w, ypos,       1.0f, 1.0f,

            xpos,     ypos + h,   0.0f, 0.0f,
            xpos + w, ypos,       1.0f, 1.0f,
            xpos + w, ypos + h,   1.0f, 0.0f
        };
        // Update polygon data for each character
        // Set the current texture
        gfxController_->bindTexture(ch.TextureID);
        gfxController_->updateBufferData(vertices, VBO);
        gfxController_->drawTriangles(6);
        x += (ch.Advance >> 6) * scale;
    }
    gfxController_->bindVao(0);
    gfxController_->bindTexture(0);
}

void TextObject::update() {
    render();
}
