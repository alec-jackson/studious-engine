/*#ifndef TEXTLOADER_HPP
#define TEXTLOADER_HPP
#include "common.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "gameObject.hpp"
#include "modelImport.hpp"

struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    ivec2   Size;       // Size of glyph
    ivec2   Bearing;    // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
};

class textLib {
  public:
    int changeFontSize(FT_Face* face, int size);
    int initText();
    int genText(FT_Face* face);
    GameObjectText *buildTextObj(string text, float x, float y, float scale,
        vec3 color);
    map<char, Character> Characters;
};

#endif
*/
