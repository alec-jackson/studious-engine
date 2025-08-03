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
#include <ft2build.h>
#include <string>
#include <vector>
#include <GameObject.hpp>
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
    explicit TextObject(string message, vec3 position, float scale, string fontPath, float charSpacing, int charPoint,
        uint programId, string objectName, ObjectType type, GfxController *gfxController);
    ~TextObject();

    // Setters
    void setMessage(string message);
    inline void setCutoff(vec3 cutoff) { cutoff_ = cutoff; }
    inline void setColor(vec4 color) { textColor_ = color; }
    inline void setColor(vec3 color) { textColor_ = vec4(color, 1.0f); }  // Compatibility
    inline void setCharPadding(float padding) { charPadding_ = padding; }

    // Getters
    inline string getMessage() { return this->message_; }
    inline vec3 getCutoff() { return cutoff_; }
    inline vec4 getColor() { return textColor_; }
    inline float getCharPadding() { return charPadding_; }

    // Render method
    void render() override;
    void update() override;
    void createMessage();
    void initializeText();
    void initializeShaderVars();
    unsigned char *rgbConversion(size_t size, unsigned char *data);

 private:
    float charPadding_;
    string message_;
    string fontPath_;
    vector<unsigned int> vaos_;
    vector<unsigned int> vbos_;
    map<char, Character> characters;

    unsigned int modelMatId_;
    unsigned int cutoffId_;
    unsigned int projectionId_;

    int charPoint_;

    mat4 modelMat_;
    vec3 cutoff_;
    vec4 textColor_;
};
