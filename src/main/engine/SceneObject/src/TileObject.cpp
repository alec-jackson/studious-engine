#include <TileObject.hpp>
#include <GL/glew.h>
#include <Image.hpp>

TileObject::TileObject(map<string, string> textures, vector<TileData> mapData, vec3 position, vec3 rotation, float scale, ObjectType type, uint programId, string objectName,
    GfxController *gfxController) : SceneObject(position, rotation, objectName, scale, programId, type, gfxController), mapData_ { mapData } {
    // Generate texture array based on the provided textures
    generateTextureData(textures);
    sanityCheck();
    processMapData();
}
void TileObject::processMapData() {
    projectionId_ = gfxController_->getShaderVariable(programId_, "projection").get();
    // Let's start with a basic triangle example
    float x = 0.0f, y = 0.0f;
    // Generate based on first texture dimensions
    float y2 = width_, x2 = height_;
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

    auto modelMatrices = std::unique_ptr<mat4[]>(new mat4[mapData_.size()]);
    auto layerIndices = std::unique_ptr<float[]>(new float[mapData_.size()]);
    auto index = 0;
    for (auto entry : mapData_) {
        // We need to create the model matrix
        mat4 model = mat4(1.0f);
        model = glm::translate(model, vec3(entry.x * width_ * scale_, entry.y * height_ * scale_, 0.0f) + position);
        model = glm::scale(model, glm::vec3(scale_));
        modelMatrices.get()[index] = model;
        // Save the current texture as an index in the texture array
        layerIndices.get()[index] = static_cast<float>(textureToIndexMap_.at(entry.texture));
        index++;
    }

    // After models generated, send the model data to OpenGL
    gfxController_->generateBuffer(&vbo);
    gfxController_->bindBuffer(vbo);
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

    gfxController_->bindBuffer(0);

    // This is going to be interesting, but add layout indices to the stream
    // Send layout index data
    gfxController_->generateBuffer(&vbo);
    gfxController_->bindBuffer(vbo);
    gfxController_->sendBufferData(sizeof(float) * mapData_.size(), &layerIndices.get()[0]);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void *)0);
    glVertexAttribDivisor(1, 1);
    assert(glGetError() == GL_NO_ERROR);

    glBindVertexArray(0);
    assert(glGetError() == GL_NO_ERROR);

    gfxController_->bindVao(0);
}

void TileObject::generateTextureData(map<string, string> textures) {
    uint currentIndex = 0;
    auto layerCount = textures.size();
    for (auto texturePath : textures) {
        SDL_Surface *surface = IMG_Load(texturePath.second.c_str());
        auto textureFormat = surface->format->Amask ? TexFormat::RGBA : TexFormat::RGB;
        auto packedPixels = packSurface(surface);
        assert(currentIndex < textures.size());
        // Use the dimensions of the first texture for the width/height of the array
        if (!currentIndex) {
            textureFormat_ = textureFormat;
            width_ = surface->w;
            height_ = surface->h;
            glGenTextures(1, &texArr_);
            assert(glGetError() == GL_NO_ERROR);
            glBindTexture(GL_TEXTURE_2D_ARRAY, texArr_);
            assert(glGetError() == GL_NO_ERROR);
            // Define the storage for the texture array
            glTexStorage3D(GL_TEXTURE_2D_ARRAY,
                1, // Mipmap level count
                textureFormat_ == TexFormat::RGB ? GL_RGB8 : GL_RGBA8, // format
                width_,
                height_,
                layerCount);
            auto error = glGetError();
            assert(error == GL_NO_ERROR);
        }
        assert(textureFormat == textureFormat_);
        assert(surface->w <= width_);
        assert(surface->h <= height_);
        // Upload the texture data
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, // Target
            0, // mipmap level
            0, // offset x
            0, // offset y
            0, // layer index
            surface->w,
            surface->h,
            layerCount,
            textureFormat_ == TexFormat::RGB ? GL_RGB : GL_RGBA,
            GL_UNSIGNED_BYTE,
            packedPixels.get());
        assert(glGetError() == GL_NO_ERROR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        textureToIndexMap_[texturePath.first] = currentIndex;
        SDL_FreeSurface(surface);
        currentIndex++;
    }
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
    //gfxController_->bindTexture(texArr_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, texArr_);
    assert(glGetError() == GL_NO_ERROR);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, mapData_.size());
    //glDrawArrays(GL_TRIANGLES, 0, 6);
    assert(glGetError() == GL_NO_ERROR);
    //glDrawArrays(GL_TRIANGLES, 0, 6);
    gfxController_->bindVao(0);
}
