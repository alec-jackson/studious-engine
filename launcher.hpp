#pragma once
#include <thread>
#include "gameInstance.hpp"

/*
gameInfo struct currently used to temporarily send camera and character data
to a function to handle input.
*/
typedef struct gameInfo {
	bool *isDone;
	GLfloat *angleX, *angleY, *angleZ;
	GLfloat *xPos, *yPos, *zPos;
	GLfloat *scale;
	GameCamera *gameCamera;
	bool *updated;
 	GameInstance currentGame;
 	mutex *infoLock;
} gameInfo;

int launch (mutex *infoLock, GameInstance *currentGame);
