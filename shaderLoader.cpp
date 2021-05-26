#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>

GLuint LoadShaders(const char* vertexShader, const char* fragmentShader) {
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    char currentChar;
    int shaderSize = 1, logLength;
    GLint success = GL_FALSE;
    FILE *fp = fopen(vertexShader, "r");
    if (fp == NULL) {
        printf("Error! Vertex shader not found!\n");
        return 0;
    }
    while (1) {
        currentChar = fgetc(fp);
        if (currentChar == EOF) {
            break;
        }
        shaderSize++;
    }
    GLchar vertexShaderData[shaderSize];
    fseek(fp, 0, SEEK_SET);
    for (int i = 0; i < shaderSize; i++) {
        currentChar = fgetc(fp);
        if (currentChar != EOF) {
            vertexShaderData[i] = currentChar;
        } else {
            vertexShaderData[i] = 0;
        }
    }
    //printf("\nVERTEX SHADER VIEW:\n\n%s\n\n", vertexShaderData);
    printf("Now compiling %s...\n", vertexShader);
    GLchar *vertShaderInfo[1];
    vertShaderInfo[0] = vertexShaderData;
    glShaderSource(vertexShaderID, 1, vertShaderInfo, NULL);
    glCompileShader(vertexShaderID);
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &success);
    glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        char errorLog[logLength + 1];
        glGetShaderInfoLog(vertexShaderID, logLength, NULL, &errorLog[0]);
        printf("%s\n", errorLog);
    }
    //printf("VertexShader stage finished.\n");
    fclose(fp);
    success = GL_FALSE;
    fp = fopen(fragmentShader, "r");
    shaderSize = 1;
    while (1) {
        currentChar = fgetc(fp);
        if (currentChar == EOF) {
            break;
        }
        shaderSize++;
    }
    GLchar fragmentShaderData[shaderSize];
    fseek(fp, 0, SEEK_SET);
    for (int i = 0; i < shaderSize; i++) {
        currentChar = fgetc(fp);
        if (currentChar != EOF) {
            fragmentShaderData[i] = currentChar;
        } else {
            fragmentShaderData[i] = 0;
        }
    }
    //printf("\nFRAGMENT SHADER VIEW:\n\n%s\n\n", fragmentShaderData);
    printf("Now compiling %s...\n", fragmentShader);
    GLchar *fragShaderInfo[1];
    fragShaderInfo[0] = fragmentShaderData;
    glShaderSource(fragmentShaderID, 1, fragShaderInfo, NULL);
    glCompileShader(fragmentShaderID);
    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &success);
    glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        char errorLog[logLength + 1];
        glGetShaderInfoLog(vertexShaderID, logLength, NULL, &errorLog[0]);
        printf("%s\n", errorLog);
    }
    //printf("FragmentShader stage finished.\n");
    fclose(fp);
    GLuint programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar errorLog[logLength + 1];
        glGetProgramInfoLog(programID, logLength, NULL, errorLog);
        printf("%s\n", errorLog);
    }
    
    glDetachShader(programID, vertexShaderID);
    glDetachShader(programID, fragmentShaderID);
    
    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);
    
    return programID;
}
/*
 int main () {
 GLuint programID = LoadShaders("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");
 
 return 0;
 }
 */
/*
 int main () {
 FILE *fp = fopen("SimpleVertexShader.vertexshader", "r");
 if (fp == NULL) {
 printf("File is missing!\n");
 return 1;
 }
 char currentChar;
 int charCount;
 while (1) {
 currentChar = fgetc(fp);
 if (currentChar == EOF) {
 break;
 }
 charCount++;
 printf("%c", currentChar);
 }
 printf("\nThere were a total of %i non-EOF characters.\n", charCount);
 currentChar = 0;
 charCount = 0;
 fseek(fp, 0, SEEK_SET);
 while (1) {
 currentChar = fgetc(fp);
 if (currentChar == EOF) {
 break;
 }
 charCount++;
 printf("%c", currentChar);
 }
 printf("\nThere were a total of %i non-EOF characters.\n", charCount);
 return 0;
 }
 */

