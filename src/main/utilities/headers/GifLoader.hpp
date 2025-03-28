/**
 * @file GifLoader.hpp
 * @author Christian Galvez
 * @brief GifLoader class for loading in GIF images as animations (image sequences). Experimental and not currently supported.
 * @copyright Copyright (c) 2025
 */
#pragma once
#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <memory>

#define GIF_HEADER_BLOCK_SIZE 6
#define GIF_LOGICAL_SCREEN_DESCRIPTOR_SIZE 7
#define GIF_IMAGE_DESCRIPTOR_SIZE 10

using std::string;
using std::vector;

enum GifVersion {
    GIFNONE,
    GIF89a,
    GIF87a,
    NUM_VERSIONS
};

struct Image {
    uint16_t imageWidth;
    uint16_t imageHeight;
    uint16_t imageLeft;
    uint16_t imageTop;
    bool localColorTableFlag;
    bool interlaceFlag;
    bool sortFlag;
    unsigned int lctSize;
    std::shared_ptr<uint8_t[]> imageData;
};

class GifLoader {
 public:
    explicit inline GifLoader(string imagePath) : imagePath_ { imagePath } { loadGif(); }
    void loadGif();
    GifVersion getVersionFromStr(const uint8_t *versionStr);
    uint16_t getCanvasWidthFromStr(const uint8_t *lsd);
    uint16_t getCanvasHeightFromStr(const uint8_t *lsd);
    uint8_t getPackedFieldFromStr(const uint8_t *lsd);
    uint8_t getBackgroundColorIndexFromStr(const uint8_t *lsd);
    uint8_t getPixelAspectRatioFromStr(const uint8_t *lsd);
    void unpackFields(uint8_t packedField);
    unsigned int processExtension(std::ifstream &inputFile);
    void processGraphicsControlExtension(std::ifstream &inputFile);
    void unpackImageDescriptor(const uint8_t *id, Image *im);
    void parseImageData(std::ifstream &inputFile);
    void lzwDecompression(uint8_t lzwMin, std::vector<uint8_t> data);
    void processColorOutputForImage(const std::vector<string> &outputData);
    unsigned int initializeColorCodeTable(uint8_t lzwMin);
    void writeBufferToImage(uint8_t *outBuffer, uint16_t fWidth, uint16_t fHeight, uint16_t iLeft,
        uint16_t iTop, Image *im);

    const Image &getImage(unsigned int imIndex) const;

    inline GifVersion getVersion() { return version_; }
    inline uint16_t getCanvasWidth() { return canvasWidth_; }
    inline uint16_t getCanvasHeight() { return canvasHeight_; }
    inline unsigned int getGlobalColorTableFlag() { return globalColorTableFlag_; }
    inline unsigned int getColorResolution() { return colorResolution_; }
    inline unsigned int getSortFlag() { return sortFlag_; }
    inline unsigned int getGlobalColorTableSize() { return globalColorTableSize_; }
    inline uint8_t getBackgroundColorIndex() { return backgroundColorIndex_; }
    inline uint8_t getPixelAspectRatio() { return pixelAspectRatio_; }

    inline uint8_t getGceBlockSize() { return gceBlockSize_; }
    inline uint16_t getGceDelayTime() { return gceDelayTime_; }
    inline uint8_t getGceTransparentColorIndex() { return gceTransparentColorIndex_; }
    inline const vector<Image> &getImages() const { return images_; }

 private:
    string imagePath_;

    uint16_t canvasWidth_;
    uint16_t canvasHeight_;

    // Don't need to be ints
    unsigned int globalColorTableFlag_;
    unsigned int colorResolution_;
    unsigned int sortFlag_;
    unsigned int globalColorTableSize_;

    // GCE variables
    uint8_t gceBlockSize_;
    uint16_t gceDelayTime_;
    uint8_t gceTransparentColorIndex_;

    uint8_t backgroundColorIndex_;
    uint8_t pixelAspectRatio_;

    GifVersion version_;

    std::shared_ptr<uint8_t> globalColorTable_;
    vector<Image> images_;
    vector<string> colorCodeTable_;
};
