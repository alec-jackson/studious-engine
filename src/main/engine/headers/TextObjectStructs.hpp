/**
 * @file TextObjectStructs.hpp
 * @author Christian Galvez
 * @brief Contains structs relevant to the TextObject class
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once
#include <string>

/**
 * @brief Used for constructing TextObjects
 * @param message(string) Message to display on the rendered text.
 * @param fontPath(string) Path to a font to use to render the text.
 * @param programId(GLuint) ProgramId for shaders to render text 
 */
typedef struct textObjectInfo {
    string message, fontPath;
    GLuint programId;
    string objectName;
    GfxController &gfxController;
} textObjectInfo;

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
