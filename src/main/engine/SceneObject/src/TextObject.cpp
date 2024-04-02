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
            GLuint textureId = gfxController_->generateFontTextures(
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                face->glyph->bitmap.buffer).get();

            Character character = {
                textureId,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };
            characters.insert(std::pair<char, Character>(c, character));
        }
        gfxController_->bindTexture(0, UINT_MAX);
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    gfxController_->initVao(&VAO);
    glGenBuffers(1, &VBO);
    gfxController_->bindVao(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    /// @todo Check if enable vertex attrib array only needs to be run once ever
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    gfxController_->bindVao(0);
    auto error = glGetError();
    if (error != 0) {
        fprintf(stderr, "TextObject::constructor: Error %d\n", error);
    }
}

/// @todo Do something useful here
TextObject::~TextObject() {
}

void TextObject::render() {
    glClear(GL_DEPTH_BUFFER_BIT);
    vec3 color = vec3(1.0f);
    int x = this->position.x, y = this->position.y;
    glUseProgram(this->programId);
    gfxController_->polygonRenderMode(RenderMode::FILL);
    glUniform3f(glGetUniformLocation(this->programId, "textColor"),
        color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);
    std::string::const_iterator c;
    for (c = message_.begin(); c != message_.end(); c++) {
        Character ch = characters[*c];
        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;
        // update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.Advance >> 6) * scale;
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    auto error = glGetError();
    if (error != 0) {
        fprintf(stderr, "TextObject::render: Error %d\n", error);
    }
}

void TextObject::update() {
    render();
}
