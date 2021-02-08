#include "launcher.hpp"
#include "shaderLoader.hpp"
#include "modelImport.hpp"
#include "gameObject.hpp"
#include "gameInstance.hpp"
#include "misc/config.hpp"

#include <stdio.h>
#include <pthread.h>

// Global Variables, should eventually be moved to a config file
const char* soundList[] = {"sfx/music/endlessNight.wav"}; // A list of gameSounds to load
const char* fragShaders[] = {"shaders/SimpleFragmentShader.fragmentshader", "shaders/coll.frag", "shaders/text.fragmentshader"}; // Contains collider renderer and basic object renderer.
const char* vertShaders[] = {"shaders/SimpleVertexShader.vertexshader", "shaders/coll.vert", "shaders/text.vertexshader"}; // Contains collider renderer and basic object renderer.

// Global Objects

int setup(pthread_mutex_t *infoLock, GameInstance *currentGame, ConfigData* config);

int main(){
  int errorNum;
  GameInstance currentGame;
  pthread_mutex_t infoLock;
  ConfigData config;

  setup(&infoLock, &currentGame, &config);

  errorNum = launch(&infoLock, &currentGame);
  pthread_mutex_destroy(&infoLock);
  return errorNum;
}

int setup(pthread_mutex_t *infoLock, GameInstance *currentGame, ConfigData* config){
  if (pthread_mutex_init(infoLock, NULL)) {
    printf("Mutex lock failed to engage!\n");
  }

  int flag = loadConfig(config, "misc/config.txt");
  if(!flag){
    currentGame -> startGameInstance(config -> resX, config -> resY, soundList, 1, vertShaders, fragShaders, 3, infoLock);
  } else{
    currentGame -> startGameInstance(1280, 720, soundList, 1, vertShaders, fragShaders, 3, infoLock);
  }


  return 0;
}
