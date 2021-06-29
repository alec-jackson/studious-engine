/*#include "textLoader.hpp"

int textLib::initText(){
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        cerr << "ERROR::FREETYPE: Could not init FreeType Library" << endl;
        return -1;
    }

    FT_Face face;
    if (FT_New_Face(ft, "misc/fonts/Cave-Story.ttf", 0, &face))
    {
        cerr << "ERROR::FREETYPE: Failed to load font" << endl;
        return -1;
    }

    //FT_Set_Pixel_Sizes(face, 0, 48);
    changeFontSize(&face, 48);

    if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
    {
        cerr << "ERROR::FREETYTPE: Failed to load Glyph" << endl;
        return -1;
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    genText(&face);
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    return 0;
}

int textLib::changeFontSize(FT_Face *face, int size){
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
            cerr << "ERROR::FREETYTPE: Failed to load Glyph" << endl;
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
            (unsigned int)(*face)->glyph->advance.x
        };
        Characters.insert(pair<char, Character>(i, character));
    }
    return 0;
}

GameObjectText *textLib::buildTextObj(string text, float x, float y, float scale, vec3 color) {

    mat4 projection = ortho(0.0f, 1280.0f, 0.0f, 720.0f);
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
*/
