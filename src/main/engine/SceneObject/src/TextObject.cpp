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
    gfxController_->initVao(&vao_);
    gfxController_->bindVao(vao_);
    auto polyCount = 6;
    poly_ = new Polygon(polyCount, programId, vector<GLfloat>(), 4);
    gfxController_->generateVertexBuffer(*poly_);
    /// @todo Check if enable vertex attrib array only needs to be run once ever
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    gfxController_->bindVao(0);
}

/// @todo Do something useful here
TextObject::~TextObject() {
}

void TextObject::render() {
    glClear(GL_DEPTH_BUFFER_BIT);
    vec3 color = vec3(1.0f);
    int x = this->position.x, y = this->position.y;
    gfxController_->setProgram(programId);
    gfxController_->polygonRenderMode(RenderMode::FILL);

    glUniform3f(glGetUniformLocation(this->programId, "textColor"),
        color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(vao_);
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
        gfxController_->bindTexture(ch.TextureID, UINT_MAX);
        gfxController_->updateBufferData(vertices, poly_->shapeBufferId[0]);
        //assert(gfxController_->render(vao_, poly_->shapeBufferId[0], UINT_MAX, UINT_MAX, poly_->pointCount[0]).isOk());
        glDrawArrays(GL_TRIANGLES, 0, 6);
        /**
         * Note for future me:
         * 
         * The render loop isn't working because it needs to constantly bind the VBO data to the VAO object for every object render...
         * This isn't necessary for static objects, and causes performance degredation. The glBindBuffer call(s) shouldn't happen from
         * the GfxController render loop, the VBO data should be tied to the VAO inside of the GameObject being rendered. This change is
         * REQUIRED to move forward, as it may cause major performance issues on embedded systems (intended target).
         */
        x += (ch.Advance >> 6) * scale;
    }
    gfxController_->bindVao(0);
    gfxController_->bindTexture(0, UINT_MAX);
}

void TextObject::update() {
    render();
}
