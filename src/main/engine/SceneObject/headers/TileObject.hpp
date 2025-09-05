/**
 * @file TileObject.hpp
 * @author Christian Galvez
 * @brief Class and method declaration for TileObject.
 * @copyright studious engine 2025
 * @date May 11, 2025
 */
#pragma once
#include <string>
#include <vector>
#include <SceneObject.hpp>
#include <GfxController.hpp>
#include <ImageExt.hpp>
#define TILE_VEC4_ATTRIBUTE_COUNT 4
#define TILE_MODEL_VEC4_START_ATTR 2
#define TILE_LAYER_FLOAT_ATTR 1

struct TileData {
    int x;
    int y;
    const char *texture;
};

class TileObject : public SceneObject, public ImageExt {
 public:
    explicit TileObject(map<string, string> textures, vector<TileData> mapData, vec3 position, vec3 rotation,
        float scale, ObjectType type, uint programId, string objectName, ObjectAnchor anchor,
        GfxController *gfxController);
    void update() override;
    void render() override;
    void *getExtension(ExtType type) override;

 private:
    void generateTextureData(map<string, string> textures);
    void processMapData();
    void sanityCheck();
    map<string, int> textureToIndexMap_;
    vector<TileData> mapData_;
    uint projectionId_;
    uint tintId_;
    uint texArr_;
    int width_;
    int height_;
    TexFormat textureFormat_;
    ObjectAnchor anchor_;
};
