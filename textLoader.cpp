#include "textLoader.hpp"
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

using namespace std;
using namespace glm;

int textLib::initText(){
  FT_Library ft;
  if (FT_Init_FreeType(&ft))
  {
      cout << "ERROR::FREETYPE: Could not init FreeType Library" << endl;
      return -1;
  }

  FT_Face face;
  if (FT_New_Face(ft, "misc/fonts/AovelSans.ttf", 0, &face))
  {
      cout << "ERROR::FREETYPE: Failed to load font" << endl;
      return -1;
  }

  //FT_Set_Pixel_Sizes(face, 0, 48);
  changeFontSize(&face, 48);

  if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
  {
      cout << "ERROR::FREETYTPE: Failed to load Glyph" << endl;
      return -1;
  }

  genText(&face);

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  FT_Done_Face(face);
  FT_Done_FreeType(ft);

  return 0;
}

int textLib::changeFontSize(FT_Face* face, int size){
    if(FT_Set_Pixel_Sizes(*face, 0, size)){
      return -1;
    }
    return 0;
}

int textLib::genText(FT_Face* face){
  unsigned int texture;
  for(unsigned char i = 0; i < 128; i++){
    // load character glyph
    if (FT_Load_Char(*face, i, FT_LOAD_RENDER)){
        cout << "ERROR::FREETYTPE: Failed to load Glyph" << endl;
        continue;
    }
    // generate texture
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        (*face)->glyph->bitmap.width,
        (*face)->glyph->bitmap.rows,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        (*face)->glyph->bitmap.buffer
    );
    // set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // now store character for later use
    Character character = {
        texture,
        ivec2((*face)->glyph->bitmap.width, (*face)->glyph->bitmap.rows),
        ivec2((*face)->glyph->bitmap_left, (*face)->glyph->bitmap_top),
        (*face)->glyph->advance.x
    };
    Characters.insert(std::pair<char, Character>(i, character));
  }

  return 0;
}

polygon *textLib::buildTextObj(string text, float x, float y, float scale, vec3 color) {
  // Create the (polygon *)model to hold the text
  polygon *model = (polygon*)malloc(sizeof(polygon));
  // Iterate through all the characters in (string)text
  std::string::const_iterator c;
  for (c = text.begin(); c != text.end(); c++)
  {
    // Set ch to the current character at c
    Character ch = Characters[*c];
    

    float xpos = x + ch.Bearing.x * scale;
    float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

    float w = ch.Size.x * scale;
    float h = ch.Size.y * scale;
    // update VBO for each character
    float vertices[6][3] = {
        { xpos,     ypos + h,   1.0f },
        { xpos,     ypos,       1.0f },
        { xpos + w, ypos,       1.0f },

        { xpos,     ypos + h,   1.0f },
        { xpos + w, ypos,       1.0f },
        { xpos + w, ypos + h,   1.0f }
    };

    // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
    x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
  }
  return 0;
}
