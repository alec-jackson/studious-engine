#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <cstring>
#ifndef SDL2_image
#include <SDL_image.h>
#endif

#include "modelImport.hpp"

void configureObject(GLfloat *vertexFrame, GLfloat *textureFrame, GLfloat *normalFrame, GLint *commands, GLint index, GLint numVertices, GLint numTextureCoords, GLint numNormals, GLint numFaces, polygon *model, int textureCount, int *texturePattern, const char **texturePath);

polygon *importObj(importObjInfo objInfo) {
    FILE *fp = fopen(objInfo.modelPath, "r");
    if (fp == NULL) {
        fprintf(stderr, "Model path does not exist!");
        return NULL;
    }
    int charBufferMax = 100, currentIndex = 0; // Assume no line in obj file has more than 30 characters
    char *charBuffer = (char*)calloc(charBufferMax, sizeof(char)), *eofCheck;
    eofCheck = charBuffer;
    if (charBuffer == NULL) {
        fprintf(stderr, "Failed to callloc!\n");
        return NULL;
    }
    float x, y, z;
    char nani;
    int numberOfObjects = 0;
    // Repeat the below process for each object
    /*
    numVertices contains the amount of distinct vertex points in the file
    numNormals contains the amount of distinct normal vectors in the file
    numTextureCoords contains the amount of distinct texture coordinate points
    in the file
    numFaces contains the amount of faces there will be in each array
    */
    int numVertices = 0, numNormals = 0, numTextureCoords = 0, numFaces = 0;
    char tempLine[3];
    tempLine[2] = 0;
    int iterations = 0, tempCount = 0;
    // Count the number of each element in the file
    while (eofCheck != NULL) {
        eofCheck = fgets(charBuffer, charBufferMax-1, fp);
        memcpy(tempLine, charBuffer, 2);
        if (strcmp(tempLine, "v ") == 0) {
            numVertices++;
        } else if (strcmp(tempLine, "vt") == 0) {
            numTextureCoords++;
        } else if (strcmp(tempLine, "vn") == 0) {
            numNormals++;
        } else if (strcmp(tempLine, "f ") == 0) {
            numFaces++;
        } else if (strcmp(tempLine, "o ") == 0) {
            numberOfObjects++;
        }
        free(charBuffer);
        charBuffer = (char*)calloc(charBufferMax, sizeof(char));
        if (charBuffer == NULL) {
            fprintf(stderr, "Failed to callloc!\n");
            return NULL;
        }
        currentIndex = 0;
    }
    // For each object, run the import code.
    // We want to run the for loop a second time so that we can allocate the
    // appropriate amount of memory needed for each struct member
    polygon *model = (polygon*)malloc(sizeof(struct polygon));
    model->programID = objInfo.programID;
    model->shapebufferID = (GLuint*)malloc(sizeof(GLuint) * numberOfObjects);
    model->normalbufferID = (GLuint*)malloc(sizeof(GLuint) * numberOfObjects);
    model->textureCoordsID = (GLuint*)malloc(sizeof(GLuint) * numberOfObjects);
    model->textureID = (GLuint*)malloc(sizeof(GLuint) * numberOfObjects);
    model->pointCount = (GLint*)malloc(sizeof(GLint) * numberOfObjects);
    model->textureCoords = (GLfloat**)malloc(sizeof(GLfloat*) * numberOfObjects);
    model->vertices = (GLfloat**)malloc(sizeof(GLfloat*) * numberOfObjects);
    model->normalCoords = (GLfloat**)malloc(sizeof(GLfloat*) * numberOfObjects);
    GLfloat vertexFrame[numVertices * 3];
    GLfloat normalFrame[numNormals * 3];
    GLfloat textureFrame[numTextureCoords * 2];
    GLint commands[numFaces*9];
    GLint currentVIndex = 0, currentTIndex = 0, currentNIndex = 0, currentCIndex = 0;
    fseek(fp, 0, SEEK_SET);
    eofCheck = charBuffer;
    int currentObject = 0; // currentObject will count as the index value + 1
    while (eofCheck != NULL) {
        eofCheck = fgets(charBuffer, charBufferMax-1, fp);
        memcpy(tempLine, charBuffer, 2);
        // Set up the current frames to be used in the configureObject call
        if (strcmp(tempLine, "v ") == 0) {
            sscanf(charBuffer, "%c %f %f %f\n", &nani, &vertexFrame[currentVIndex*3], &vertexFrame[currentVIndex * 3 + 1], &vertexFrame[currentVIndex * 3 + 2]);
            currentVIndex++;
        } else if (strcmp(tempLine, "vt") == 0) {
            sscanf(charBuffer, "vt %f %f\n", &textureFrame[currentTIndex*2], &textureFrame[currentTIndex*2+1]);
            currentTIndex++;
        } else if (strcmp(tempLine, "vn") == 0) {
            sscanf(charBuffer, "vn %f %f %f\n", &normalFrame[currentNIndex*3], &normalFrame[currentNIndex*3+1], &normalFrame[currentNIndex*3+2]);
            currentNIndex++;
        } else if (strcmp(tempLine, "f ") == 0) {
            sscanf(charBuffer, "%c %i/%i/%i %i/%i/%i %i/%i/%i\n", &nani, &commands[currentCIndex*9], &commands[currentCIndex*9+1], &commands[currentCIndex*9+2], &commands[currentCIndex*9+3], &commands[currentCIndex*9+4], &commands[currentCIndex*9+5], &commands[currentCIndex*9+6], &commands[currentCIndex*9+7], &commands[currentCIndex*9+8]);
            currentCIndex++;
        // When we hit the first obect, ignore
        } else if (strcmp(tempLine, "o ") == 0) {
            // When currentObject == 0, run the following
            if (!currentObject) {
                currentObject++;
            } else {
                configureObject(vertexFrame, textureFrame, normalFrame,
                    commands, currentObject-1, currentVIndex, currentTIndex,
                    currentNIndex, currentCIndex, model, objInfo.numTextures,
                    objInfo.texturePattern, objInfo.texturePath);
                currentObject++;
            }
        }
        free(charBuffer);
        charBuffer = (char*)calloc(charBufferMax, sizeof(char));
        if (charBuffer == NULL) {
            fprintf(stderr, "Failed to callloc!\n");
            return NULL;
        }
        currentIndex = 0;
    }
    // Send the final object to be configured in the function.
    configureObject(vertexFrame, textureFrame, normalFrame, commands,
        currentObject-1, currentVIndex, currentTIndex, currentNIndex,
        currentCIndex, model, objInfo.numTextures, objInfo.texturePattern,
        objInfo.texturePath);
    free(charBuffer);
    model->textureUniformID = glGetUniformLocation(objInfo.programID,
        "mytexture");
    model->numberOfObjects = numberOfObjects;
    return model;
}

void configureObject(GLfloat *vertexFrame, GLfloat *textureFrame, GLfloat *normalFrame, GLint *commands, GLint index, GLint numVertices, GLint numTextureCoords, GLint numNormals, GLint numFaces, polygon *model, int numTextures, int *texturePattern, const char **texturePath) {
    model->textureCoords[index] = NULL;
    model->pointCount[index] = numFaces;
    GLfloat *vertexVBO = (GLfloat*)calloc(numFaces*3*3, sizeof(GLfloat));
    GLfloat *textureVBO = (GLfloat*)calloc(numFaces*3*2, sizeof(GLfloat));
    GLfloat *normalVBO = (GLfloat*)calloc(numFaces*3*3, sizeof(GLfloat));
    // Current assignment index for vertex, normal and texture coords.
    int caiv = 0, caino = 0, cait = 0, currentCommandIndex = 0, currentCommand = 0;
    for (int i = 0; i < numFaces; i++) {
        for (int k = 0; k < 3; k++) {
            currentCommandIndex = (i*9) + (k*3); // vertex coord command
            currentCommand = commands[currentCommandIndex];
            // Assign vertex data
            for (int l = 0; l < 3; l++) {
                vertexVBO[caiv++] = vertexFrame[(currentCommand-1)*3+l];
            }
            currentCommandIndex = (i*9) + (k*3) + 1; // textureCoord command
            currentCommand = commands[currentCommandIndex];
            textureVBO[cait++] = textureFrame[(currentCommand-1)*2];
            textureVBO[cait++] = 1.0f - textureFrame[(currentCommand-1)*2+1];
            currentCommandIndex = (i*9) + (k*3) + 2; // normal command
            currentCommand = commands[currentCommandIndex];
            for (int l = 0; l < 3; l++) {
                normalVBO[caino++] = normalFrame[(currentCommand-1)*3+l];
            }
        }
    }
    model->vertices[index] = vertexVBO;
    model->textureCoords[index] = textureVBO;
    model->normalCoords[index] = normalVBO;
    glGenBuffers(1, &model->shapebufferID[index]);
    glBindBuffer(GL_ARRAY_BUFFER, model->shapebufferID[index]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * model->pointCount[index] * 9, model->vertices[index], GL_STATIC_DRAW);
    glGenBuffers(1, &model->normalbufferID[index]);
    glBindBuffer(GL_ARRAY_BUFFER, model->normalbufferID[index]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * model->pointCount[index] * 9, model->normalCoords[index], GL_STATIC_DRAW);
    // Specific case where the current object does not get a texture
    if (numTextures == 0 || texturePattern[index] >= numTextures || texturePattern[index] == -1) {
        model->textureCoords[index] = NULL;
        return;
    }
    SDL_Surface* texture = IMG_Load(texturePath[texturePattern[index]]);
    if (texture == NULL) {
        return;
    }
    glGenTextures(1, &model->textureID[index]);
    glBindTexture(GL_TEXTURE_2D, model->textureID[index]);
    if (texture->format->Amask) {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->w, texture->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->pixels);
    } else {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->w, texture->h, 0, GL_RGB, GL_UNSIGNED_BYTE, texture->pixels);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 10);
    glGenerateMipmap(GL_TEXTURE_2D);
    SDL_FreeSurface(texture);
    //glGenerateMipmap(GL_TEXTURE_2D);
    glGenBuffers(1, &model->textureCoordsID[index]);
    glBindBuffer(GL_ARRAY_BUFFER, model->textureCoordsID[index]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * model->pointCount[index] * 6, model->textureCoords[index], GL_STATIC_DRAW);
    //printf("Getting uniform location\n");

    //printf("OpenGL commands worked successfully in modelImport.\n");
    return;
}

//polygon *createSprite(imagePath, programID) {

//}
