#pragma once

#include <stdio.h>
#include <iostream>
#include <map>
#include <GL/glew.h>
#include <ft2build.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include FT_FREETYPE_H

#include "gameObject.hpp"
#include "modelImport.hpp"

using namespace glm;
using namespace std;

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
    polygon *buildTextObj(string text, float x, float y, float scale, vec3 color);

    map<char, Character> Characters;
};
