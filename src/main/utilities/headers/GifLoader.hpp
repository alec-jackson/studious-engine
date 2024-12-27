#pragma once
#include <string>
#include <vector>
#include <map>

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
    unsigned short imageWidth;
    unsigned short imageHeight;
    bool localColorTableFlag;
    bool interlaceFlag;
    bool sortFlag;
    unsigned int lctSize;
};

class GifLoader {
public:
    explicit inline GifLoader(string imagePath) : imagePath_ { imagePath } { loadGif(); }
    void loadGif();
    GifVersion getVersionFromStr(const byte *versionStr);
    unsigned short getCanvasWidthFromStr(const byte *lsd);
    unsigned short getCanvasHeightFromStr(const byte *lsd);
    byte getPackedFieldFromStr(const byte *lsd);
    byte getBackgroundColorIndexFromStr(const byte *lsd);
    byte getPixelAspectRatioFromStr(const byte *lsd);
    void unpackFields(byte packedField);
    unsigned int processExtension(std::ifstream &inputFile);
    void processGraphicsControlExtension(std::ifstream &inputFile);
    void unpackImageDescriptor(const byte *id, Image *im);
    void parseImageData(std::ifstream &inputFile);
    void lzwDecompression(byte lzwMin, std::vector<byte> data);

    inline GifVersion getVersion() { return version_; }
    inline unsigned short getCanvasWidth() { return canvasWidth_; }
    inline unsigned short getCanvasHeight() { return canvasHeight_; }
    inline unsigned int getGlobalColorTableFlag() { return globalColorTableFlag_; }
    inline unsigned int getColorResolution() { return colorResolution_; }
    inline unsigned int getSortFlag() { return sortFlag_; }
    inline unsigned int getGlobalColorTableSize() { return globalColorTableSize_; }
    inline byte getBackgroundColorIndex() { return backgroundColorIndex_; }
    inline byte getPixelAspectRatio() { return pixelAspectRatio_; }

    inline byte getGceBlockSize() { return gceBlockSize_; }
    inline unsigned short getGceDelayTime() { return gceDelayTime_; }
    inline byte getGceTransparentColorIndex() { return gceTransparentColorIndex_; }
    inline vector<Image> getImages() { return images_; }
private:
    string imagePath_;

    unsigned short canvasWidth_;
    unsigned short canvasHeight_;

    // Don't need to be ints
    unsigned int globalColorTableFlag_;
    unsigned int colorResolution_;
    unsigned int sortFlag_;
    unsigned int globalColorTableSize_;

    // GCE variables
    byte gceBlockSize_;
    unsigned short gceDelayTime_;
    byte gceTransparentColorIndex_;

    byte backgroundColorIndex_;
    byte pixelAspectRatio_;

    GifVersion version_;

    byte *globalColorTable_ = nullptr;
    vector<Image> images_;
    vector<string> colorCodeTable_;
};
