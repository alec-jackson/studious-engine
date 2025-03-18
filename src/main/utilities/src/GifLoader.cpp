/**
 * @file GifLoader.cpp
 * @brief GifLoader class implementation
 * @author Christian Galvez
 * @copyright Copyright (c) 2025
 */
#include <GifLoader.hpp>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

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
        auto gctSize = 1 << (globalColorTableSize_ + 1);
        gctSize *= 3;
        // Delete this when the object is destroyed
        globalColorTable_ = new byte[gctSize];
        for (int i = 0; i < gctSize; ++i) {
            char readByte;
            inputFile.get(readByte);
            globalColorTable_[i] = readByte;
        }
        // Start creating each image
        while (!inputFile.eof()) {
            // Now we need to process any annoying extensions that
            // may exist in the file...
            // auto extensionSize = 0;
            // While the next byte is 21, iterate through until it's not
            char extCode;
            inputFile.get(extCode);
            if (extCode == 0x3B) {
                printf("GifLoader::loadGif: End of file reached\n");
                break;
            }
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

            // Start reading image data
            parseImageData(inputFile);
        }
    } else {
        printf("Failed to open file\n");
    }
    inputFile.close();

    return;
}

void GifLoader::parseImageData(std::ifstream &inputFile) {
    // Fail the tests
    pixelAspectRatio_ = 1;
     // Actually parse image data now...
    char lzwMinByte;
    inputFile.get(lzwMinByte);
    byte lzwMin = lzwMinByte;

    printf("GifLoader::lzwMin: %u\n", lzwMin);

    char readByte;
    std::vector<byte> data;
    // Subblock loop
    while (inputFile.get(readByte)) {
        // This outer loop will check for the subblockSize
        byte subblockSize = readByte;
        printf("GifLoader::parseImageData: subblock size: %u\n", subblockSize);
        // If the subblockSize is zero, then we break early because we're done reading data
        if (subblockSize == 0x00) break;

        // Otherwise, start another loop to read the entire subblock
        for (byte i = 0; i < subblockSize; ++i) {
            inputFile.get(readByte);
            // Add the read byte to the data array
            data.push_back(readByte);
        }
    }
    lzwDecompression(lzwMin, data);
    printf("IMAGE DATA START\n");
    for (auto i : data) {
        printf("%02x ", i);
    }
    printf("\nIMAGE DATA END\n");
}

int GifLoader::initializeColorCodeTable(byte lzwMin) {
    // Clear out the color code table to reset it
    colorCodeTable_.clear();
    // Generate the code table using the lzw min
    int numberOfColors = 1 << lzwMin;
    for (int i = 0; i < numberOfColors; ++i) {
        colorCodeTable_.push_back(std::to_string(i));
    }

    // Add the Clear Code and End of Information Code
    colorCodeTable_.push_back("CC");
    colorCodeTable_.push_back("EOIC");

    return numberOfColors;
}

void GifLoader::lzwDecompression(byte lzwMin, vector<byte> data) {
    auto numberOfColors = initializeColorCodeTable(lzwMin);
    auto clearCodeIndex = numberOfColors;
    auto endOfInfoIndex = numberOfColors + 1;
    auto grabBits = [&data](unsigned int bitSize, unsigned int addr) {
        // Ensure bitsize is 12 or below
        assert(bitSize <= 12);
        // Shift by address - determine number of bytes we need
        auto divAddr = addr / 8;
        auto remAddr = addr % 8;

        // Grab an entire integer from the data array (bounds check please)
        unsigned int result = 0;
        unsigned int dataSize = data.size();
        for (unsigned int i = 0; i < 4; ++i) {
            auto elem = (3 - i) + divAddr;
            // Check if the data array has an element at i + divAddr
            if (elem >= dataSize) {
                // printf("End of array reached, not adding to result\n");
                // assert(i < 3);
                continue;
            }
            // Add to the current result (32 bits)
            result <<= 8;
            result += data.at(elem);
        }
        // First we want to downshift the result by remAddr
        result >>= remAddr;
        // Then we just mask the number of bits we want and return it
        return result & ((1 << bitSize) - 1);
    };
    vector<string> outputCodes;
    auto currentBitSize = lzwMin + 1;
    auto address = 0;
    auto currentRead = 0;
    auto currentCode = grabBits(currentBitSize, address);
    string nextCode;
    bool impartialOutput = false;
    bool initialRead = false;
    bool resetBitSize = false;
    currentRead++;
    // The first code we process should be the clear code
    assert(currentCode == clearCodeIndex);
    while (currentCode != endOfInfoIndex) {
        // printf("GifLoader::lzwDecompression: Processing code: %u\n", currentCode);
        // Process the current code
        if (currentCode == clearCodeIndex) {
            printf("GifLoader::lzwDecompression: Found clear code\n");
            // Reset the color code table
            initializeColorCodeTable(lzwMin);
            // Reset the current bit size
            // currentBitSize = lzwMin + 1;
            currentRead = 1;
            resetBitSize = true;
            initialRead = false;
        } else {
            // Perform code processing here
            // Read value
            // This should always be true -- we just write partial values
            assert(currentCode < colorCodeTable_.size());
            auto decodedValue = colorCodeTable_.at(currentCode);
            if (initialRead) {
                // Modify the tail end of the colorCodeTable with the first char of decoded val
                auto lastCode = colorCodeTable_.back();
                lastCode += std::string(";") + decodedValue.substr(0, 1);
                colorCodeTable_[colorCodeTable_.size() - 1] = lastCode;
                if (impartialOutput) {
                    impartialOutput = false;
                    // Update the partial output to make it whole
                    outputCodes[outputCodes.size() - 1] = lastCode;
                }
                // Update the decoded value
                decodedValue = colorCodeTable_.at(currentCode);
            }
            // We write the code to the table as impartial ALWAYS - will finish this later
            colorCodeTable_.push_back(decodedValue);
            // If the currentCode is the last entry in the color table, then we're doing an impartial output
            if (currentCode == colorCodeTable_.size() - 2) {
                impartialOutput = false;
            }
            outputCodes.push_back(decodedValue);
            initialRead = true;
        }
        currentRead++;
        // Increment address
        address += currentBitSize;
        if (resetBitSize) {
            currentBitSize = lzwMin + 1;
            resetBitSize = false;
        }
        if (currentRead > (1 << (currentBitSize - 1))) {
            currentBitSize++;
            currentRead = 1;
        }
        // read the next code at the start of the next loop
        currentCode = grabBits(currentBitSize, address);
    }

    // We need to simultaneously load data from the file as we decode using lzw
    // Issue bits to decode stream

    // Now convert the raw output to pixel data using the gct
    processColorOutputForImage(outputCodes);
}

void GifLoader::processColorOutputForImage(const std::vector<string> &outputData) {
    Image &im = images_.back();
    bool fullImageFlush = false;
    auto width = im.imageWidth;
    auto height = im.imageHeight;
    // Check if this is a partial image flush or not
    // The image should be width * height
    if (im.imageTop == 0 && im.imageLeft == 0) {
        // This is a full image - do normal stuff
        fullImageFlush = true;
    }
    if (fullImageFlush == true) {
        printf("Full image flush detected\n");
    } else {
        printf("Partial image flush detected\n");
        assert(images_.size() > 1);
        // Set the width/height to the previous frame (should be complete)
        width = images_.at(images_.size() - 2).imageWidth;
        height = images_.at(images_.size() - 2).imageHeight;
    }
    std::unique_ptr<byte[]> outBuffer(new byte[im.imageWidth * im.imageHeight * 3]);
    // Ensure we have a previous full image to use as a reference
    im.imageData = new byte[width * height * 3];
    int currentColor = 0;
    auto processOutput = [&] (string &outString, byte *outBuffer) {
        auto out = std::stoi(outString);
        // Copy the three corresponding color bytes from the gct into the image data
        memcpy(&outBuffer[currentColor * 3], &globalColorTable_[out * 3], sizeof(byte) * 3);
        currentColor++;
    };
    for (auto out : outputData) {
        assert(out.empty() == false);
        string s;
        for (auto c : out) {
            if (c == ';') {
                // Process the output string
                processOutput(s, outBuffer);
                // clear string
                s.clear();
            } else {
                s += c;
            }
        }
        assert(s.empty() == false);
        // Process the output string s here again
        processOutput(s, outBuffer);
    }
    // If we're doing a full image flush, then copy outbuffer to image data
    if (fullImageFlush) {
        memcpy(im.imageData, outBuffer, sizeof(outBuffer));
    } else {
        // Copy the last frame into the current image buffer
        memcpy(im.imageData, images_.at(images_.size() - 2).imageData, sizeof(byte) * width * height * 3);

        // Draw the output buffer on top of the previous frame
        writeBufferToImage(outBuffer, width, height, im.imageLeft, im.imageTop, &im);
    }
    // Update width/height values for subsequent images...
    im.imageWidth = width;
    im.imageHeight = height;
    printf("GifLoader::processColorOutputForImage: Complete!\n");
}

void GifLoader::writeBufferToImage(byte *outBuffer, uint16_t fWidth, uint16_t fHeight, uint16_t iLeft,
    uint16_t iTop, Image *im) {
    // Base case
    if (iTop - im->imageTop == im->imageHeight) return;
    // Recursive function to write to image buffer
    // Find the index in the buffer where we start writing
    auto startIdx = (iTop * fWidth + iLeft) * 3;
    // Perform a memcpy for the entire line from the output buffer
    memcpy(&im->imageData[startIdx], outBuffer, sizeof(byte) * im->imageWidth * 3);
    // Start the next iteration
    return writeBufferToImage(&outBuffer[im->imageWidth * 3], fWidth, fHeight, iLeft, iTop + 1, im);
}

void GifLoader::unpackImageDescriptor(const byte *id, Image *im) {
    // Make sure the first byte is 2C
    assert(id[0] == 0x2C);

    // Bytes 1-2 are the image left position
    im->imageLeft = (id[2] << 8) | id[1];
    // Bytes 3-4 are the image top position
    im->imageTop = (id[4] << 8) | id[3];
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

const Image &GifLoader::getImage(int imIndex) const {
    assert(imIndex >= 0 && imIndex < images_.size());
    return images_.at(imIndex);
}

uint16_t GifLoader::getCanvasWidthFromStr(const byte *lsd) {
    uint16_t width = 0;
    // Sanity check
    if (lsd == nullptr) return width;
    // The canvas width is stored in the first two bytes (little endian)
    width = (lsd[1] << 8) | lsd[0];
    return width;
}

uint16_t GifLoader::getCanvasHeightFromStr(const byte *lsd) {
    uint16_t height = 0;
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
    printf("GifLoader::processExtension: Processing extension %02x\n", (byte)extensionLabel);

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
    // I think the int cast here is to prevent overflows, but I can't remember now...
    // Might be causing the rendering issue - investigate later if desired
    std::unique_ptr<char[]> appId(new char[static_cast<int>(blockSize) + 1]);
    appId[static_cast<int>(blockSize)] = '\0';

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
        std::unique_ptr<char[]> debugData(new char[byteCount + 1]);
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
    // assert(terminator == 0x00);

    // assert(terminator == '\0');
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

    // NOLINTNEXTLINE
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
