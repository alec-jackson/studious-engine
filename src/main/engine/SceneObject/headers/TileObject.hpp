/**
 * @file TileObject.hpp
 * @author Christian Galvez
 * @brief Class and method declaration for TileObject.
 * @copyright studious engine 2025
 * @date May 11, 2025
 */
#pragma once
#include <SceneObject.hpp>
#include <GfxController.hpp>

struct TileData {
    int x;
    int y;
    const char *texture;
};

class TileObject : public SceneObject {
 public:
    explicit TileObject(vector<string> textures, vector<TileData> mapData, vec3 position, vec3 rotation, float scale, ObjectType type, uint programId, string objectName, GfxController *gfxController);
    void sanityCheck();
    void update() override;
    void render() override;
 private:
    void basicTriangle();
    void generateTextureData(vector<string> textures);
    void processMapData();
    map<string, int> textureToIndexMap_;
    vector<TileData> mapData_;
    uint projectionId_;
};

