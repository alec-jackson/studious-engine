#include <TileObject.hpp>
#include <GL/glew.h>

TileObject::TileObject(vector<string> textures, vector<TileData> mapData, vec3 position, vec3 rotation, float scale, ObjectType type, uint programId, string objectName,
    GfxController *gfxController) : SceneObject(position, rotation, objectName, scale, programId, type, gfxController), mapData_ { mapData } {
    // What the flip
    // Generate texture array based on the provided textures
    //generateTextureData(textures);
    //sanityCheck();
    //scale_ = 10;
    //processMapData();
    basicTriangle();
}
void TileObject::basicTriangle() {
    projectionId_ = gfxController_->getShaderVariable(programId_, "projection").get();
    // Let's start with a basic triangle example
    float x = 0.0f, y = 0.0f;
    float y2 = 100.0f, x2 = 100.0f;
    vector<float> vertData = {
        x, y2, 0.0f, 0.0f,
        x, y, 0.0f, 1.0f,
        x2, y2, 1.0f, 0.0f,

        x2, y2, 1.0f, 0.0f,
        x, y, 0.0f, 1.0f,
        x2, y, 1.0f, 1.0f
    };

    // Create the VAO
    gfxController_->initVao(&vao_);
    gfxController_->bindVao(vao_);

    // Create VBO and send coord data
    uint vbo;
    gfxController_->generateBuffer(&vbo);
    gfxController_->bindBuffer(vbo);

    gfxController_->sendBufferData(sizeof(float) * vertData.size(), vertData.data());
    gfxController_->enableVertexAttArray(0, 4);
    gfxController_->bindBuffer(0);
    gfxController_->bindVao(0);
}

void TileObject::generateTextureData(vector<string> textures) {
    uint currentIndex = 0;
    uint texture;
    auto width = 0, height = 0;
    auto layerCount = textures.size();
    for (auto texturePath : textures) {
        SDL_Surface *surface = IMG_Load(texturePath.c_str());
        assert(currentIndex < textures.size());
        // Use the dimensions of the first texture for the width/height of the array
        if (!currentIndex) {
            width = surface->w;
            height = surface->h;
            glGenTextures(1, &texture);
            assert(glGetError() == GL_NO_ERROR);
            glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
            assert(glGetError() == GL_NO_ERROR);
            // Define the storage for the texture array
            glTexStorage3D(GL_TEXTURE_2D_ARRAY,
                1, // Mipmap level count
                GL_RGBA8, // format
                width,
                height,
                layerCount);
            auto error = glGetError();
            assert(error == GL_NO_ERROR);
        }
        assert(surface->w <= width);
        assert(surface->h <= height);
        // Upload the texture data
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, // Target
            0, // mipmap level
            0, // offset x
            0, // offset y
            0, // layer index
            surface->w,
            surface->h,
            layerCount,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            surface->pixels);
        assert(glGetError() == GL_NO_ERROR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        textureToIndexMap_[texturePath] = currentIndex;
        SDL_FreeSurface(surface);
        currentIndex++;
    }
}

void TileObject::processMapData() {
    // What do we actually need to do?
    auto x = 0.0f, y = 0.0f;
    auto x2 = x + (16), y2 = y + (16);
    // update VBO for each character
    // UV coordinate origin STARTS in the TOP left, NOT BOTTOM LEFT!!!
    vector<float> vertData = {
        x, y2, 0.0f, 0.0f,
        x, y, 0.0f, 1.0f,
        x2, y2, 1.0f, 0.0f,

        x2, y2, 1.0f, 0.0f,
        x, y, 0.0f, 1.0f,
        x2, y, 1.0f, 1.0f
    };
    uint vbo;
    gfxController_->initVao(&vao_);
    gfxController_->bindVao(vao_);
    gfxController_->generateBuffer(&vbo);
    gfxController_->bindBuffer(vbo);
    gfxController_->sendBufferData(sizeof(vertData), vertData.data());
    gfxController_->enableVertexAttArray(0, 4);
    // We can probably just keep the vao bound for now.
    // Model data for each tile
    auto modelMatrices = std::unique_ptr<mat4[]>(new mat4[mapData_.size()]);
    auto layerIndices = std::unique_ptr<float[]>(new float[mapData_.size()]);
    // 1. We need to map the texture strings to a specific layer.
    for (auto entry : mapData_) {
        printf("TileObject::processMapData: Calculating model data stream\n");
        // Get the layer index
        auto ttimit = textureToIndexMap_.find(entry.texture);
        // This assert will occur if the texture map data references a texture not supplied
        assert(ttimit != textureToIndexMap_.end());
        auto index = ttimit->second;

        // We need to create the model matrix

        mat4 model = mat4(1.0f);
        model = glm::translate(model, vec3(entry.x, entry.y, 0.0f));
        model = glm::scale(model, glm::vec3(scale_));
        modelMatrices.get()[index] = model;

        // Save the layer index for the texture used
        layerIndices.get()[index] = static_cast<float>(index);
    }
    // 2. We need to use some standard base coordinates to create the vertex data and texture coords. Do something similar to sprite object.

    // 3. Need to apply model transformations per object. Apply scale, translation, rotation. Model data is streamed as an attrib pointer.

    // 4. Generate VAO with vertex data VBO for ONE OBJECT. Will use instancing for drawing.

    // 5. We need to stream the layer index with the texture coordinates. I think this can be done using an attrib pointer. Don't repeat numbers for same obj.
    uint buffer;
    // Send layout index data
    gfxController_->generateBuffer(&buffer);
    gfxController_->bindBuffer(buffer);
    gfxController_->sendBufferData(sizeof(float) * mapData_.size(), &layerIndices.get()[0]);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void *)0);
    glVertexAttribDivisor(1, 1);
    assert(glGetError() == GL_NO_ERROR);
    // I tihnk it's fine to keep the VAO Bound????
    gfxController_->generateBuffer(&buffer);
    gfxController_->bindBuffer(buffer);
    gfxController_->sendBufferData(mapData_.size() * sizeof(mat4), &modelMatrices.get()[0]);
    // We need to generate a vec4 for each model
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (void *)0);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (void *)(1 * sizeof(vec4)));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (void *)(2 * sizeof(vec4)));
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(vec4), (void *)(3 * sizeof(vec4)));
    assert(glGetError() == GL_NO_ERROR);

    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    assert(glGetError() == GL_NO_ERROR);
    glVertexAttribDivisor(5, 1);
    assert(glGetError() == GL_NO_ERROR);

    glBindVertexArray(0);
    assert(glGetError() == GL_NO_ERROR);
    // 6. ???
    projectionId_ = gfxController_->getShaderVariable(programId_, "projection").get();
}

void TileObject::sanityCheck() {
    // make sure none of the tiles use a texture we aren't expecting
    for (auto entry : mapData_) {
        assert(textureToIndexMap_.find(entry.texture) != textureToIndexMap_.end());
    }
}

void TileObject::update() {
    render();
}

void TileObject::render() {
    // No additional model updates will be performed. This is a one-and-done thing.
    gfxController_->clear(GfxClearMode::DEPTH);
    gfxController_->setProgram(programId_);
    gfxController_->polygonRenderMode(RenderMode::FILL);
    gfxController_->sendFloatMatrix(projectionId_, 1, glm::value_ptr(vpMatrix_));
    assert(glGetError() == GL_NO_ERROR);
    gfxController_->bindVao(vao_);
    assert(glGetError() == GL_NO_ERROR);
    //glDrawArraysInstanced(GL_TRIANGLES, 0, 6, mapData_.size());
    glDrawArrays(GL_TRIANGLES, 0, 6);
    assert(glGetError() == GL_NO_ERROR);
    //glDrawArrays(GL_TRIANGLES, 0, 6);
    gfxController_->bindVao(0);
}
