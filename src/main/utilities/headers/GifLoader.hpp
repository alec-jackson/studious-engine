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

#define GIF_HEADER_BLOCK_SIZE 6
#define GIF_LOGICAL_SCREEN_DESCRIPTOR_SIZE 7
#define GIF_IMAGE_DESCRIPTOR_SIZE 10

using std::string;
using std::vector;

typedef unsigned char byte;

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
    byte *imageData;
};

class GifLoader {
 public:
    explicit inline GifLoader(string imagePath) : imagePath_ { imagePath } { loadGif(); }
    void loadGif();
    GifVersion getVersionFromStr(const byte *versionStr);
    uint16_t getCanvasWidthFromStr(const byte *lsd);
    uint16_t getCanvasHeightFromStr(const byte *lsd);
    byte getPackedFieldFromStr(const byte *lsd);
    byte getBackgroundColorIndexFromStr(const byte *lsd);
    byte getPixelAspectRatioFromStr(const byte *lsd);
    void unpackFields(byte packedField);
    unsigned int processExtension(std::ifstream &inputFile);
    void processGraphicsControlExtension(std::ifstream &inputFile);
    void unpackImageDescriptor(const byte *id, Image *im);
    void parseImageData(std::ifstream &inputFile);
    void lzwDecompression(byte lzwMin, std::vector<byte> data);
    void processColorOutputForImage(const std::vector<string> &outputData);
    int initializeColorCodeTable(byte lzwMin);
    void writeBufferToImage(byte *outBuffer, uint16_t fWidth, uint16_t fHeight, uint16_t iLeft,
        uint16_t iTop, Image *im);

    const Image &getImage(int imIndex) const;

    inline GifVersion getVersion() { return version_; }
    inline uint16_t getCanvasWidth() { return canvasWidth_; }
    inline uint16_t getCanvasHeight() { return canvasHeight_; }
    inline unsigned int getGlobalColorTableFlag() { return globalColorTableFlag_; }
    inline unsigned int getColorResolution() { return colorResolution_; }
    inline unsigned int getSortFlag() { return sortFlag_; }
    inline unsigned int getGlobalColorTableSize() { return globalColorTableSize_; }
    inline byte getBackgroundColorIndex() { return backgroundColorIndex_; }
    inline byte getPixelAspectRatio() { return pixelAspectRatio_; }

    inline byte getGceBlockSize() { return gceBlockSize_; }
    inline uint16_t getGceDelayTime() { return gceDelayTime_; }
    inline byte getGceTransparentColorIndex() { return gceTransparentColorIndex_; }
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
    byte gceBlockSize_;
    uint16_t gceDelayTime_;
    byte gceTransparentColorIndex_;

    byte backgroundColorIndex_;
    byte pixelAspectRatio_;

    GifVersion version_;

    byte *globalColorTable_ = nullptr;
    vector<Image> images_;
    vector<string> colorCodeTable_;
};
