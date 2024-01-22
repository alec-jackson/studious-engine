/**
 * @file OpenGlGfxController.cpp
 * @author Christian Galvez
 * @brief 
 * @version 0.1
 * @date 2024-01-21
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <OpenGlGfxController.hpp>

GfxResult OpenGlGfxController::generateVertexBuffer(Polygon &polygon) {
    cout << "OpenGlGfxController::generateVertexBuffer" << endl;
    glGenBuffers(1, &(polygon.shapeBufferId[0]));
    glBindBuffer(GL_ARRAY_BUFFER, polygon.shapeBufferId[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat) * polygon.pointCount[0] * 9,
                 &(polygon.vertices[0][0]), GL_STATIC_DRAW);
    /// @todo? Error check
    return GfxResult::OK;
}

GfxResult OpenGlGfxController::generateNormalBuffer(Polygon &polygon) {
    cout << "OpenGlGfxController::generateNormalBuffer" << endl;
    glGenBuffers(1, &(polygon.normalBufferId[0]));
    glBindBuffer(GL_ARRAY_BUFFER, polygon.normalBufferId[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat) * polygon.pointCount[0] * 9,
                 &(polygon.normalCoords[0][0]), GL_STATIC_DRAW);
    return GfxResult::OK;
}

GfxResult OpenGlGfxController::generateTextureBuffer(Polygon &polygon, SDL_Surface *texture) {
    cout << "OpenGlGfxController::generateTextureBuffer" << endl;
    if (texture == nullptr) return GfxResult::FAILURE;
    glGenTextures(1, &(polygon.textureId[0]));
    glBindTexture(GL_TEXTURE_2D, polygon.textureId[0]);
    if (texture->format->Amask) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, texture->pixels);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->w, texture->h, 0, GL_RGB,
                     GL_UNSIGNED_BYTE, texture->pixels);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_NEAREST_MIPMAP_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 10);
    glGenerateMipmap(GL_TEXTURE_2D);

    // glGenerateMipmap(GL_TEXTURE_2D);
    glGenBuffers(1, &(polygon.textureCoordsId[0]));
    glBindBuffer(GL_ARRAY_BUFFER, polygon.textureCoordsId[0]);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat) * polygon.pointCount[0] * 6,
                 &(polygon.textureCoords[0][0]), GL_STATIC_DRAW);

    return GfxResult::OK;
}
