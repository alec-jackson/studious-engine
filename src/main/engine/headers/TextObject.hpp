#pragma once
#include "common.hpp"
#include "ModelImport.hpp"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "TextObjectStructs.hpp"


class TextObject: public GameObject {
    public:
        // Constructors
        TextObject(textObjectInfo info);
        ~TextObject();

        // Setters
        inline void setMessage(string message) { this->message = message; };

        // Getters
        inline string getMessage() { return this->message; };

        // Render method
        void render();
    private:
        string message;
        int fontSize;
        unsigned int VAO, VBO; // TODO: Where are these being used? Protected at a level above?
        unsigned int textureUniformId; // TODO: Investigate whether this can be protected  at a level above
        map<GLchar, Character> characters;    
};
