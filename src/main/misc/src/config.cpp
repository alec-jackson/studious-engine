/**
 * @file config.cpp
 * @author Alec Jackson, Christian Galvez
 * @brief Configures runtime application settings
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <string>
#include <cstdio>
#include <iostream>
#include <config.hpp>

/*
 (int) loadConfig takes a (configData *) config file and populates it with
 configuration items found in the (string) filename file.

 On success, 0 is returned and teh config struct is populated with data. On
 failure, 1 is returned and the config struct is not populated with data.
*/
int loadConfig(configData* config, string filename) {
    SDL_RWops* file;
    file = SDL_RWFromFile(filename.c_str(), "r");
    if (file) {
        char buf[512];
        char convert[11];
        int varSaves[3];
        int retSize = 0;
        int loops = 3;
        int offset = 0;
        retSize = SDL_RWread(file, buf, sizeof(char), 1024);
        for (int j = 0; j < loops; j++) {
            for (int i = offset; i < retSize + offset; i++) {
                if (buf[i] == '=') {
                    int start = i + 1;
                    i++;
                    while (isdigit(buf[i])) {
                        i++;
                    }
                    int end = i + 1;
                    int dif = end - start;
                    if (dif > 10) {
                        dif = 10;
                    }
                    for (int k = 0; k < dif; k++) {
                        convert[k] = buf[k + start];
                    }
                    convert[dif] = '\n';
                    offset += end;
                    break;
                }
            }
            varSaves[j] = atoi(convert);
        }
        SDL_RWclose(file);
        printf("loadConfig: Resolution X: %d, Resolution Y: %d, VSYNC: %d\n",
            varSaves[0], varSaves[1], varSaves[2]);
        config->resX = varSaves[0];
        config->resY = varSaves[1];
        config->enableVsync = static_cast<bool>(varSaves[2]);
        return 0;
    } else {
        cout << "Could not open config file\n";
        return 1;
    }
}
