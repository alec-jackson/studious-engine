#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <cstring>

#include "modelImport.hpp"

objImp *importObj(const char* modelPath) {
    FILE *fp = fopen(modelPath, "r");
    if (fp == NULL) {
        fprintf(stderr, "Model path does not exist!");
        return NULL;
    }
    int charBufferMax = 40, currentIndex = 0; // Assume no line in obj file has more than 30 characters
    char *charBuffer = (char*)calloc(charBufferMax, sizeof(char)), *eofCheck;
    eofCheck = charBuffer;
    if (charBuffer == NULL) {
        fprintf(stderr, "Failed to callloc!\n");
        return NULL;
    }
    float x, y, z;
    char nani;
    int numVertices = 0;
    char vertexLine[3];
    vertexLine[2] = 0;
    int iterations = 0;
    while (eofCheck != NULL) {
        eofCheck = fgets(charBuffer, 39, fp);
        memcpy(vertexLine, charBuffer, 2);
        if (strcmp(vertexLine, "v ") == 0) {
            //printf("%s", charBuffer);
            //sscanf(charBuffer, "%c %f %f %f\n", &nani, &x, &y, &z);
            //printf("vertex: x = %f, y = %f, z = %f\n", x, y, z);
            numVertices++;
        }
        free(charBuffer);
        charBuffer = (char*)calloc(charBufferMax, sizeof(char));
        if (charBuffer == NULL) {
            fprintf(stderr, "Failed to callloc!\n");
            return NULL;
        }
        currentIndex = 0;
    }
    printf("numVertices = %d\n", numVertices);
    // Create struct and malloc vertex data
    objImp *model = (objImp*)malloc(sizeof(struct objImp));
    model->numVertices = numVertices;
    model->data = (GLfloat*)malloc(sizeof(GLfloat) * numVertices * 3);
    int currentVertex = 0;
    fseek(fp, 0, SEEK_SET);
    eofCheck = charBuffer;
    // Do it again because of laziness
    while (eofCheck != NULL) {
        eofCheck = fgets(charBuffer, 39, fp);
        memcpy(vertexLine, charBuffer, 2);
        if (strcmp(vertexLine, "v ") == 0) {
            //printf("%s", charBuffer);
            sscanf(charBuffer, "%c %f %f %f\n", &nani, &model->data[currentVertex * 3], &model->data[(currentVertex * 3) + 1], &model->data[(currentVertex * 3) + 2]);
            //printf("vertex: x = %f, y = %f, z = %f\n", model->data[currentVertex * 3], model->data[(currentVertex * 3) + 1], model->data[(currentVertex * 3) + 2]);
            currentVertex++;
        }
        free(charBuffer);
        charBuffer = (char*)calloc(charBufferMax, sizeof(char));
        if (charBuffer == NULL) {
            fprintf(stderr, "Failed to callloc!\n");
            return NULL;
        }
        currentIndex = 0;
    }
    for (int i = 0; i < numVertices; i++) {
        printf("vertex: x = %f, y = %f, z = %f\n", model->data[i * 3], model->data[(i * 3) + 1], model->data[(i * 3) + 2]);
    }
    return model;
}

int main () {
    GLint nate = 0;
    printf("This is working.\n");
    importObj("shreksan.obj");
    return 0;
}
