/**
 * @file Image.hpp
 * @author Christian Galvez
 * @brief Image class definition for storing image data
 * @version 0.1
 * @date 2024-10-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once
#include <vector>

struct Image {
    /* Constraints across all image resolutions */
    int width;
    int height;

    /* textureIds for each frame */
    std::vector<unsigned int> textureIds;
};
