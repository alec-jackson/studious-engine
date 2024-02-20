/**
 * @file TextObject.hpp
 * @author Christian Galvez
 * @brief TextObject is a GameObject; orthographically rendered on-screen text object
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once
#include <string>
#include <GameObject.hpp>
#include <ft2build.h> //NOLINT
#include FT_FREETYPE_H

/**
 * @brief Font details to feed into freetype2
 * @param TextureID(unsigned_int) ID handle of the glyph texture
 * @param Size(ivec2) Size of the glyph
 * @param Bearing(ivec2) Offset from baseline to left/top of glyph
 * @param Advance(unsigned_int) Offset to advance to next glyph
 */
typedef struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    ivec2 Size;       // Size of glyph
    ivec2 Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
} Character;

class TextObject: public SceneObject {
 public:
        // Constructors
        explicit TextObject(string message, string fontPath, GLuint programId, string objectName,
              GfxController *gfxController);
        ~TextObject();

        // Setters
        inline void setMessage(string message) { this->message_ = message; }

        // Getters
        inline string getMessage() { return this->message_; }

        // Render method
        void render();
        void update();
 private:
        string message_;
        string fontPath_;
        int fontSize;
        unsigned int VAO, VBO;  /// @todo: Where are these being used? Protected at a level above?
        unsigned int textureUniformId;  /// @todo: Investigate whether this can be protected  at a level above
        map<GLchar, Character> characters;
        ViewMode viewMode;
};
