#pragma once
#include <pthread.h>
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
  pthread_mutex_t *infoLock;
} gameInfo;

int launch (pthread_mutex_t *infoLock, GameInstance *currentGame);
