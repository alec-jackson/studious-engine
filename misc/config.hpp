#include "../common.hpp"

/*
 configData contains data parsed from a configuration file for the purposes of
 creating an new SDL window instance. The following struct members are described
 below:
 * (int) resX - Width of the SDL window in pixels.
 * (int) resY - Height of the SDL window in pixels.
*/
typedef struct configData {
  int resX;
  int resY;
} configData;

int loadConfig(configData* config, string filename);
