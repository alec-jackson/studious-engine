#include <TileObject.hpp>
#include <GL/glew.h>
#include <Image.hpp>

TileObject::TileObject(map<string, string> textures, vector<TileData> mapData, vec3 position, vec3 rotation, float scale, ObjectType type, uint programId, string objectName,
    ObjectAnchor anchor, GfxController *gfxController) : SceneObject(position, rotation, objectName, scale, programId, type, gfxController), mapData_ { mapData },
    anchor_ { anchor }, tint_ { vec3(0) } {
    // Generate texture array based on the provided textures
    generateTextureData(textures);
    sanityCheck();
    processMapData();
}
void TileObject::processMapData() {
    projectionId_ = gfxController_->getShaderVariable(programId_, "projection").get();
    tintId_ = gfxController_->getShaderVariable(programId_, "tint").get();
    // Let's start with a basic triangle example
    float x = 0.0f, y = 0.0f;
    switch (anchor_) {
        case BOTTOM_LEFT:
            x = 0.0f;
            y = 0.0f;
            break;
        case CENTER:
            x = -1 * ((width_) / 2.0f);
            y = -1 * ((height_) / 2.0f);
            break;
        case TOP_LEFT:
            y = -1.0f * height_;
            x = 0.0f;
            break;
        default:
            fprintf(stderr, "TileObject::processMapData: Unsupported anchor type %d\n", anchor_);
            assert(false);
            break;
    }
    // Generate based on first texture dimensions
    auto x2 = x + (width_), y2 = y + (height_);
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
    gfxController_->enableVertexAttArray(0, 4, sizeof(float), 0);
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
    // We need to do something ABSOLUTELY CANCER Here. ES 2.0 does not support divisors, so we
    // need to duplicate data by a factor of 4
    // Duplicate each model matrix 4 times in sequence, so each model matrix is like
    // 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2 and so on...
    auto modelMatricesDup = std::unique_ptr<mat4[]>(new mat4[mapData_.size() * 4]);
    auto layerIndicesDup = std::unique_ptr<float[]>(new float[mapData_.size() * 4]);
    for (int i = 0; i < mapData_.size(); i++) {
        auto modelIndex = i * 4;
        auto layerIndex = i * 4;
        modelMatricesDup.get()[modelIndex] = modelMatrices.get()[i];
        modelMatricesDup.get()[modelIndex + 1] = modelMatrices.get()[i];
        modelMatricesDup.get()[modelIndex + 2] = modelMatrices.get()[i];
        modelMatricesDup.get()[modelIndex + 3] = modelMatrices.get()[i];
        layerIndicesDup.get()[layerIndex] = layerIndices.get()[i];
        layerIndicesDup.get()[layerIndex + 1] = layerIndices.get()[i];
        layerIndicesDup.get()[layerIndex + 2] = layerIndices.get()[i];
        layerIndicesDup.get()[layerIndex + 3] = layerIndices.get()[i];
    }
    // Now re-assign the model matrices to the duplicated data
    modelMatrices = std::move(modelMatricesDup);
    layerIndices = std::move(layerIndicesDup);

    // After models generated, send the model data to OpenGL
    gfxController_->generateBuffer(&vbo);
    gfxController_->bindBuffer(vbo);
    gfxController_->sendBufferData(mapData_.size() * sizeof(mat4), &modelMatrices.get()[0]);
    for (int i = 0; i < TILE_VEC4_ATTRIBUTE_COUNT; i++) {
        auto layout = TILE_MODEL_VEC4_START_ATTR + i;
        gfxController_->enableVertexAttArray(layout, 4, sizeof(vec4), (void *)(i * sizeof(vec4)));
        gfxController_->setVertexAttDivisor(layout, 1);
    }
    gfxController_->bindBuffer(0);

    // This is going to be interesting, but add layout indices to the stream
    // Send layout index data
    gfxController_->generateBuffer(&vbo);
    gfxController_->bindBuffer(vbo);
    gfxController_->sendBufferData(sizeof(float) * mapData_.size(), &layerIndices.get()[0]);
    gfxController_->enableVertexAttArray(TILE_LAYER_FLOAT_ATTR, 1, sizeof(float), 0);
    gfxController_->setVertexAttDivisor(TILE_LAYER_FLOAT_ATTR, 1);
    gfxController_->bindVao(0);
}

void TileObject::generateTextureData(map<string, string> textures) {
    uint currentIndex = 0;
    auto layerCount = textures.size();
    for (auto texturePath : textures) {
        SDL_Surface *surface = IMG_Load(texturePath.second.c_str());
        if (surface == nullptr) {
            fprintf(stderr, "TileObject::generateTextureData: Cannot open texture file %s\n",
                texturePath.second.c_str());
            assert(0);
        }
        // RGBA texture formatting will be REQUIRED
        auto textureFormat = surface->format->Amask ? TexFormat::RGBA : TexFormat::RGB;
        if (textureFormat == TexFormat::RGB) {
            auto convertedSurface = convertSurfaceToRgba(surface);
            if (convertedSurface != nullptr) {
                surface = convertedSurface;
                textureFormat = TexFormat::RGBA;
            }
        }
        auto packedPixels = packSurface(surface);
        assert(currentIndex < textures.size());
        // Use the dimensions of the first texture for the width/height of the array
        if (!currentIndex) {
            textureFormat_ = textureFormat;
            width_ = surface->w;
            height_ = surface->h;
            gfxController_->generateTexture(&texArr_);
            gfxController_->bindTexture(texArr_, GfxTextureType::ARRAY);
            // Define the storage for the texture array
            gfxController_->allocateTexture3D(textureFormat_, width_, height_, layerCount);
        }
        assert(textureFormat == textureFormat_);
        assert(surface->w <= width_);
        assert(surface->h <= height_);
        assert(currentIndex < layerCount);
        // Upload the texture data
        gfxController_->sendTextureData3D(
            0,
            0,
            currentIndex,
            surface->w,
            surface->h,
            textureFormat_,
            packedPixels.get());
        gfxController_->setTexParam(TexParam::WRAP_MODE_S, TexVal(TexValType::CLAMP_TO_EDGE), GfxTextureType::ARRAY);
        gfxController_->setTexParam(TexParam::WRAP_MODE_T, TexVal(TexValType::CLAMP_TO_EDGE), GfxTextureType::ARRAY);
        gfxController_->setTexParam(TexParam::MAGNIFICATION_FILTER, TexVal(TexValType::NEAREST_NEIGHBOR), GfxTextureType::ARRAY);
        gfxController_->setTexParam(TexParam::MINIFICATION_FILTER, TexVal(TexValType::NEAREST_NEIGHBOR), GfxTextureType::ARRAY);
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
    gfxController_->sendFloatVector(tintId_, 1, glm::value_ptr(tint_));
    gfxController_->sendFloatMatrix(projectionId_, 1, glm::value_ptr(vpMatrix_));
    gfxController_->bindVao(vao_);
    gfxController_->bindTexture(texArr_, GfxTextureType::ARRAY);
    gfxController_->drawTrianglesInstanced(6, mapData_.size());
    gfxController_->bindVao(0);
}
