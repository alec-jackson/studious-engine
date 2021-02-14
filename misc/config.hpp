#include <SDL.h>

typedef struct ConfigData {
  int resX;
  int resY;
} ConfigData;

int loadConfig(ConfigData* config, const char* filename);
