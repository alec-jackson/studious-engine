#include <SDL.h>
#include "config.hpp"

int loadConfig(ConfigData* config, char* filename){
  SDL_RWops* file;
  file = SDL_RWFromFile(filename, "r");
  if(file){
    char buf[512];
    char convert[11];
    int varSaves[2];
    int retSize = 0;
    int loops = 2;
    int offset = 0;
    retSize = SDL_RWread(file, buf, sizeof(char), 1024);
    for(int j = 0; j < loops; j++){
      for(int i = offset; i < retSize + offset; i++){
        if(buf[i] == '='){
          int start = i + 1;
          i++;
          while(isdigit(buf[i])){ i++; }
          int end = i + 1;
          int dif = end - start;
          if(dif > 10){
            dif = 10;
          }
          for(int k = 0; k < dif; k++){
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
    printf("Resolution X: %d Resolution Y: %d \n", varSaves[0], varSaves[1]);
    config -> resX = varSaves[0];
    config -> resY = varSaves[1];

    return 0;
  } else {
    printf("Could not open config file\n");
    return 1;
  }
}
