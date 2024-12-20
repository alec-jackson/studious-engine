#include <GifLoader.hpp>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <assert.h>

const char *versionStrings[] = {
    "NO_GIF_ERROR",
    "GIF89a",
    "GIF87a",
    nullptr
};

void GifLoader::loadGif() {
    // check if the imagePath is set
    if (imagePath_.empty()) {
        printf("GifLoader::loadGif: No image provided, exiting.\n");
    }

    // Attempt to open the image at the provided path
    std::ifstream inputFile(imagePath_);
    // Todo - move these into their own functions
    byte headerBlock[GIF_HEADER_BLOCK_SIZE];
    byte logicalScreenDesc[GIF_LOGICAL_SCREEN_DESCRIPTOR_SIZE];

    if (inputFile.is_open()) {
        for (int i = 0; i < GIF_HEADER_BLOCK_SIZE; ++i) {
            // Write the header to the headerBlock
            char readByte;
            inputFile.get(readByte);
            headerBlock[i] = readByte;
        }
        // Grab the version string from the file header
        version_ = getVersionFromStr(headerBlock);
        // The next seven bytes are the Logical Screen Descriptor
        for (int i = 0; i < GIF_LOGICAL_SCREEN_DESCRIPTOR_SIZE; ++i) {
            char readByte;
            inputFile.get(readByte);
            logicalScreenDesc[i] = readByte;
        }

        // Grab the width from the LSD
        canvasWidth_ = getCanvasWidthFromStr(logicalScreenDesc);
        canvasHeight_ = getCanvasHeightFromStr(logicalScreenDesc);
        auto packedField = getPackedFieldFromStr(logicalScreenDesc);
        unpackFields(packedField);
        backgroundColorIndex_ = getBackgroundColorIndexFromStr(logicalScreenDesc);
        pixelAspectRatio_ = getPixelAspectRatioFromStr(logicalScreenDesc);
        // Make sure we have a global color table...
        assert(globalColorTableFlag_ == 1);
        // Calculate the size of the global color table
        auto gctSize = 1 << (colorResolution_ + 1);
        gctSize *= 3;
        assert(gctSize == 768);
        // Delete this when the object is destroyed
        globalColorTable_ = new byte[gctSize];
        for (int i = 0; i < gctSize; ++i) {
            char readByte;
            inputFile.get(readByte);
            globalColorTable_[i] = readByte;
        }
        // Now we need to process any annoying extensions that
        // may exist in the file...
        //auto extensionSize = 0;
        // While the next byte is 21, iterate through until it's not
        char extCode;
        inputFile.get(extCode);
        while ((byte)extCode == 0x21) {
            // Undo the seek
            inputFile.seekg(-1, std::ios::cur);
            processExtension(inputFile);
            // Read the extCode again
            inputFile.get(extCode);
        }
        inputFile.seekg(-1, std::ios::cur);

        // Read the image descripter header
        byte imageDescriptorHeader[GIF_IMAGE_DESCRIPTOR_SIZE];
        for (int i = 0; i < GIF_IMAGE_DESCRIPTOR_SIZE; ++i) {
            char readByte;
            inputFile.get(readByte);
            imageDescriptorHeader[i] = readByte;
        }
        Image im;
        unpackImageDescriptor(imageDescriptorHeader, &im);
        images_.push_back(im);
        // If a lct is used, we would parse it here :)
        assert(im.localColorTableFlag == false);

    } else {
        printf("Failed to open file\n");
    }
    inputFile.close();

    return;
}

void GifLoader::parseImageData(std::ifstream &inputFile) {
     // Actually parse image data now...
    char lzwMinByte;
    inputFile.get(lzwMinByte);
    byte lzwMin = lzwMinByte;

    // Subblock loop
    char subblockSize;
    inputFile.get(subblockSize);
    while (subblockSize != 0x00) {
        // Read image data from subblock...
        
    }
}

void GifLoader::unpackImageDescriptor(const byte *id, Image *im) {
    // Make sure the first byte is 2C
    assert(id[0] == 0x2C);

    // Bytes 1-4 are obsolete, so we can ignore them
    // Image width is little endian bytes 5-6
    im->imageWidth = (id[6] << 8) | id[5];
    // Height is bytes 7-8
    im->imageHeight = (id[8] << 8) | id[7];
    // Unpack byte 9 for other fields
    im->localColorTableFlag = id[9] & 0b10000000;
    im->interlaceFlag = id[9] & 0b01000000;
    im->sortFlag = id[9] & 0b00100000;
    im->lctSize = id[9] & 0b00000111;
}

void GifLoader::unpackFields(byte packedField) {
    // The most significant bit is the global color table flag
    globalColorTableFlag_ = (packedField & 0b10000000) >> 7;
    // The next three bits are the color resolution
    colorResolution_ = (packedField & 0b01110000) >> 4;
    // The next bit is the sort flag
    sortFlag_ = (packedField & 0b00001000) >> 3;
    // The last three bits are the size of global color table
    globalColorTableSize_ = packedField & 0b00000111;
}

GifVersion GifLoader::getVersionFromStr(const byte *str) {
    GifVersion gifVersion = GIFNONE;
    if (str == nullptr) return gifVersion;
    // Check if the str matches any of the supported versions
    for (int i = 0; i < GifVersion::NUM_VERSIONS; ++i) {
        int result = memcmp(str, versionStrings[i], GIF_HEADER_BLOCK_SIZE);
        if (result == 0) {
            // Set the GifVersion accordingly
            gifVersion = static_cast<GifVersion>(i);
        }
    }
    return gifVersion;
}

unsigned short GifLoader::getCanvasWidthFromStr(const byte *lsd) {
    unsigned short width = 0;
    // Sanity check
    if (lsd == nullptr) return width;
    // The canvas width is stored in the first two bytes (little endian)
    width = (lsd[1] << 8) | lsd[0];
    return width;
}

unsigned short GifLoader::getCanvasHeightFromStr(const byte *lsd) {
    unsigned short height = 0;
    // Sanity check
    if (lsd == nullptr) return height;
    // Canvas height is stored in bytes 2 and 3 (starting at zero)
    height = (lsd[3] << 8) | lsd[2];
    return height;
}

byte GifLoader::getPackedFieldFromStr(const byte *lsd) {
    byte packedField = 0;
    // Sanity check
    if (lsd == nullptr) return packedField;
    // the packed field will be byte 4 in the LSD
    packedField = lsd[4];
    return packedField;
}

byte GifLoader::getBackgroundColorIndexFromStr(const byte *lsd) {
    byte backgroundColorIndex = 0;
    if (lsd == nullptr) return backgroundColorIndex;
    backgroundColorIndex = lsd[5];
    return backgroundColorIndex;
}

byte GifLoader::getPixelAspectRatioFromStr(const byte *lsd) {
    byte pixelAspectRatio = 0;
    if (lsd == nullptr) return pixelAspectRatio;
    pixelAspectRatio = lsd[6];
    return pixelAspectRatio;
}

unsigned int GifLoader::processExtension(std::ifstream &inputFile) {
    auto ignoreExtension = false;
    auto isNetscapeExt = false;
    // Make sure the extensionIntroducer is 0x21
    char extensionIntroducer;
    inputFile.get(extensionIntroducer);
    assert(extensionIntroducer == 0x21);

    char extensionLabel;
    inputFile.get(extensionLabel);
    
    printf("GifLoader::processExtension: Processing extension %02x\n", extensionLabel);

    if ((byte)extensionLabel != 0xF9) {
        printf("GifLabeL::processExtension: Ignoring non-graphics control extension\n");
        ignoreExtension = true;
    } else {
        // Start processing the GCE
        processGraphicsControlExtension(inputFile);
        return 0;
    }

    char blockSize;
    inputFile.get(blockSize);

    printf("GifLoader::processExtension: blockSize = %u\n", blockSize);

    // use the block size to read the next name
    char appId[(int)blockSize + 1];
    appId[(int)blockSize] = '\0';

    for (int i = 0; i < blockSize; ++i) {
        char readByte;
        inputFile.get(readByte);
        appId[i] = readByte;
    }

    if (memcmp(appId, "NETSCAPE2.0", blockSize) == 0) {
        printf("NETSCAPE EXTENSION FOUND, TRIGGERING SPECIAL BEHAVIOR\n");
        isNetscapeExt = true;
    }

    printf("GifLoader::processExtension: appId = %s\n", appId);
    if (isNetscapeExt) {
        char bytesAfter;
        inputFile.get(bytesAfter);

        printf("GifLoader::processExtension: Skipping the next %u bytes\n", bytesAfter);

        // Seek past the number of bytes
        inputFile.seekg(bytesAfter, std::ios::cur);

    } else {
        // Just find the next null terminator character
        char readByte = 0xFF;
        auto byteCount = 1;
        while (inputFile.get(readByte) && readByte != 0x00) {
            byteCount++;
        }
        printf("GifLoader::processExtension: Extension data is %d bytes long\n", byteCount);
        char debugData[byteCount + 1];
        debugData[byteCount] = 0;
        // Seek backwards
        inputFile.seekg(-1 * byteCount, std::ios::cur);
        // Now read those bytes into the buffer
        for (int i = 0; i < byteCount; ++i) {
            inputFile.get(readByte);
            debugData[i] = readByte;
        }

        printf("GifLoader::processExtension: Extension data = %s\n", debugData);
    }
    
    // Make sure the next byte we read is the terminator
    char terminator;
    inputFile.get(terminator);
    printf("GifLoader::processExtension: Final char is %02x\n", terminator);
    //assert(terminator == 0x00);
    

    //assert(terminator == '\0');
    printf("IgnoreExtensionFlag: %u\n", ignoreExtension);
    return 0;
}

void GifLoader::processGraphicsControlExtension(std::ifstream &inputFile) {
    // We're entering this function right after reading the F9 byte
    // First byte is the total block size in bytes
    char blockSize;
    inputFile.get(blockSize);

    // Set the block size
    gceBlockSize_ = (byte) blockSize;

    // Next byte is the packed field
    char packedField;
    inputFile.get(packedField);

    // Unpack the gce fields here
    // Actually lol lets see if we even need them

    char delayTime[2];
    inputFile.get(delayTime[0]);
    inputFile.get(delayTime[1]);

    gceDelayTime_ = (delayTime[1] << 8) | delayTime[0];

    char transparentColorIndex;
    inputFile.get(transparentColorIndex);

    gceTransparentColorIndex_ = (byte) transparentColorIndex;

    char blockTerminator;
    inputFile.get(blockTerminator);

    printf("GifLoader::processGraphicsControlExtension: blockSize: %02x, packedField: %02x, delayTime: %02x %02x, tci: %02x, blockTerm: %02x\n",
        blockSize,
        packedField,
        delayTime[0],
        delayTime[1],
        transparentColorIndex,
        blockTerminator);

    assert(blockTerminator == 0x00);
    return;
}