/*#include "modelImport.hpp"

void configureObject(configureArgs args);


void configureObject(configureArgs args) {
    cout << "Configuring Object\n";
    args.model->pointCount.push_back(args.commands.size() / 9);
    vector<GLfloat> vertexVBO;
    vector<GLfloat> textureVBO;
    vector<GLfloat> normalVBO;
    // Iterate over each polygon in model
    for (int i = 0; i < args.model->pointCount[args.index]; i++) {
        for (int k = 0; k < 3; k++) { // Loop through each point
            int currentCommandIndex = (i*9) + (k*3); // vertex coord command
            int currentCommand = args.commands[currentCommandIndex];
            // Assign vertex data
            for (int l = 0; l < 3; l++) {
                vertexVBO.push_back(args.vertexFrame[(currentCommand-1)*3+l]);
            }
            currentCommandIndex = (i*9) + (k*3) + 1; // textureCoord command
            currentCommand = args.commands[currentCommandIndex];
            if (args.textureFrame.size() > 0) {
                textureVBO.push_back(args.textureFrame[(currentCommand-1)*2]);
                textureVBO.push_back(1.0f - args.textureFrame[(currentCommand-1)*2+1]);
            } else {
                textureVBO.push_back(0.0f);
                textureVBO.push_back(0.0f); // Add dummy values for missing data
            }
            currentCommandIndex = (i*9) + (k*3) + 2; // normal command
            currentCommand = args.commands[currentCommandIndex];
            for (int l = 0; l < 3; l++) {
                normalVBO.push_back(args.normalFrame[(currentCommand-1)*3+l]);
            }
        }
    }
    args.model->vertices.push_back(vertexVBO);
    args.model->textureCoords.push_back(textureVBO);
    args.model->normalCoords.push_back(normalVBO);
    args.model->shapebufferID.push_back(0); // Create space on vector for buffer
    args.model->normalbufferID.push_back(0); // Create space for normal buff
    // Texture values will default to UINT_MAX to signify no texture
    args.model->textureID.push_back(UINT_MAX);
    args.model->textureCoordsID.push_back(UINT_MAX);
    glGenBuffers(1, &(args.model->shapebufferID[args.index]));
    glBindBuffer(GL_ARRAY_BUFFER, args.model->shapebufferID[args.index]);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat) * args.model->pointCount[args.index] * 9,
                 &(args.model->vertices[args.index][0]), GL_STATIC_DRAW);
    glGenBuffers(1, &(args.model->normalbufferID[args.index]));
    glBindBuffer(GL_ARRAY_BUFFER, args.model->normalbufferID[args.index]);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat) * args.model->pointCount[args.index] * 9,
                 &(args.model->normalCoords[args.index][0]), GL_STATIC_DRAW);
    // Specific case where the current object does not get a texture
    if (!args.textureCount || args.texturePattern[args.index] >= args.textureCount || args.texturePattern[args.index] == -1) {
        return;
    }
    SDL_Surface *texture = IMG_Load(args.texturePath[args.texturePattern[args.index]].c_str());
    if (texture == NULL) {
        cerr << "Failed to create SDL_Surface texture!\n";
        return;
    }
    glGenTextures(1, &(args.model->textureID[args.index]));
    glBindTexture(GL_TEXTURE_2D, args.model->textureID[args.index]);
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
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 10);
    glGenerateMipmap(GL_TEXTURE_2D);
    SDL_FreeSurface(texture);
    //glGenerateMipmap(GL_TEXTURE_2D);
    glGenBuffers(1, &(args.model->textureCoordsID[args.index]));
    glBindBuffer(GL_ARRAY_BUFFER, args.model->textureCoordsID[args.index]);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(GLfloat) * args.model->pointCount[args.index] * 6,
                 &(args.model->textureCoords[args.index][0]), GL_STATIC_DRAW);
}
*/