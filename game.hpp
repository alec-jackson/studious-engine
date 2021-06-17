#ifndef GAME_HPP
#define GAME_HPP
#include "shaderLoader.hpp"
#include "modelImport.hpp"
#include "gameObject.hpp"
#include "gameInstance.hpp"
#include "misc/config.hpp"
#include "inputMonitor.hpp"
#include "physics.hpp"
#include "textLoader.hpp"
#include "common.hpp"

/*
gameInfo struct currently used to temporarily send camera and character data
to a function to handle input.
*/
typedef struct gameInfo {
	bool *isDone;
	GameCamera *gameCamera;
 	GameInstance *currentGame;
} gameInfo;

#endif
