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
#include <TextObjectStructs.hpp>


class TextObject: public GameObject {
 public:
        // Constructors
        explicit TextObject(textObjectInfo info);
        ~TextObject();

        // Setters
        inline void setMessage(string message) { this->message = message; }

        // Getters
        inline string getMessage() { return this->message; }

        // Render method
        void render();
 private:
        string message;
        int fontSize;
        unsigned int VAO, VBO;  /// @todo: Where are these being used? Protected at a level above?
        unsigned int textureUniformId;  /// @todo: Investigate whether this can be protected  at a level above
        map<GLchar, Character> characters;
};
