#include <SDL2/SDL.h>

typedef struct ConfigData {
  int resX;
  int resY;
} ConfigData;

int loadConfig(ConfigData* config, char* filename);
