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
#include <vector>
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
    ivec2 Size;              // Size of glyph
    ivec2 Bearing;           // Offset from baseline to left/top of glyph
    unsigned int Advance;    // Offset to advance to next glyph
} Character;

class TextObject : public SceneObject {
 public:
    // Constructors
    /// @todo Remove ObjectType - we render by camera now, so this isn't really needed...
    explicit TextObject(string message, vec3 position, float scale, string fontPath, unsigned int programId,
        string objectName, ObjectType type, GfxController *gfxController);
    ~TextObject() override;

    // Setters
    void setMessage(string message);

    // Getters
    inline string getMessage() { return this->message_; }

    // Render method
    void render() override;
    void update() override;
    void createMessage();
    void initializeText();
    void initializeShaderVars();
    unsigned char *rgbConversion(size_t size, unsigned char *data);

 private:
    string message_;
    string fontPath_;
    vector<unsigned int> vaos_;
    vector<unsigned int> vbos_;
    map<char, Character> characters;

    unsigned int modelMatId_;
    unsigned int cutoffId_;

    mat4 modelMat_;
    vec3 cutoff_;
};
