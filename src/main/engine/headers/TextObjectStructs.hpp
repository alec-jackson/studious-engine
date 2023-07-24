/**
 * @brief Used for constructing TextObjects
 * @param message(string) Message to display on the rendered text.
 * @param fontPath(string) Path to a font to use to render the text.
 * @param programId(GLuint) ProgramId for shaders to render text 
 */
typedef struct textObjectInfo {
	string message, fontPath;
	GLuint programId;
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