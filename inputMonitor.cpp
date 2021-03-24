#include <stdio.h>
#include <GL/glew.h>
#include <unistd.h>
#include <pthread.h>

#include "gameInstance.hpp"
#include "launcher.hpp"
#include "physics.hpp"

//Analog joystick dead zone
const int JOYSTICK_DEAD_ZONE = 4000;
#define PI 3.14159265

/*
  THIS IS A REFERENCE FILE
*/

void* rotateShape(void *gameInfoStruct) {
  printf("Fork successful\n");
	gameInfo *currentGameInfo = (gameInfo*)gameInfoStruct;
  GameInstance currentGame = currentGameInfo->currentGame;
  pthread_mutex_t *infoLock = currentGameInfo->infoLock;
	GLfloat speed = 0.001f, rotateSpeed = 1.0f, scaleSpeed = 0.0002f, offsetSpeed = 0.1f;
	GLfloat cameraOffset[3] = {5.140022f, 1.349999f, 0}, currentLuminance = 1.0f;
	int maxFrames = 1000, currentFrame = 0;
	int dx, dy;
	GLfloat frameTimes[maxFrames], angles[3] = {*currentGameInfo->angleX, *currentGameInfo->angleY, *currentGameInfo->angleZ}, pos[3] = {*currentGameInfo->xPos, *currentGameInfo->yPos, *currentGameInfo->zPos}, currentScale = *currentGameInfo->scale;
	int oldMouseX = 0, oldMouseY = 0;
	int mouseX, mouseY;
	float fallspeed = 0;
  short trackMouse = 1;

  SDL_GameController* gameController1 = NULL;
  int numJoySticks = SDL_NumJoysticks();
  short hasActiveController = 0;
  if(numJoySticks < 1){
    printf("No joysticks connected, continuing without joysticks");
  } else {
    for(int i = 0; i < numJoySticks; i++){
      if(SDL_IsGameController(i)){
        gameController1 = SDL_GameControllerOpen(i);
        if(gameController1 != 0){
          hasActiveController = 1;
          break;
        }
      }
    }
  }

  Sint16 controllerLeftStateY = 0;
  Sint16 controllerLeftStateX = 0;
  Sint16 controllerRightStateY = 0;
  Sint16 controllerRightStateX = 0;
  Uint8 buttonCheckA =  0;

	while (!*currentGameInfo->isDone) {
		//printf("The current mouse position is x:%i y:%i\n", mouseX, mouseY);
		SDL_GetRelativeMouseState(&mouseX, &mouseY);
    if(hasActiveController){
      controllerLeftStateY = SDL_GameControllerGetAxis(gameController1, SDL_CONTROLLER_AXIS_LEFTY );
      controllerLeftStateX = SDL_GameControllerGetAxis(gameController1, SDL_CONTROLLER_AXIS_LEFTX );
      controllerRightStateY = SDL_GameControllerGetAxis(gameController1, SDL_CONTROLLER_AXIS_RIGHTY );
      controllerRightStateX = SDL_GameControllerGetAxis(gameController1, SDL_CONTROLLER_AXIS_RIGHTX );
      buttonCheckA = SDL_GameControllerGetButton(gameController1, SDL_CONTROLLER_BUTTON_A );
    }
		//SDL_WarpMouseInWindow(currentGame.getWindow(), currentGame.getWidth() / 2, currentGame.getHeight() / 2);
		usleep(9000);
    //Begin Camera Controls
		if(currentGame.getKeystate()[SDL_SCANCODE_KP_5]) {
			cameraOffset[0] = 5.140022f; // Set values back to inital values
			cameraOffset[1] = 1.349999f;
			cameraOffset[2] = 0;
		}
		if (currentGame.getKeystate()[SDL_SCANCODE_KP_2] || mouseY < 0 && trackMouse || controllerRightStateY > JOYSTICK_DEAD_ZONE) {
			float modifier = 1;
			if(mouseY < 0){
				modifier = (mouseY / 5) * -1;
			} else if(controllerRightStateY > JOYSTICK_DEAD_ZONE){
        modifier = ((float)controllerRightStateY) / 32767;
      }
			double distHold = cameraOffset[1] * cameraOffset[1] + cameraOffset[2] * cameraOffset[2];
			double distHold2 = cameraOffset[1] * cameraOffset[1] + cameraOffset[0] * cameraOffset[0];
			cameraOffset[1] -= offsetSpeed * modifier;
			double distFinish = cameraOffset[1] * cameraOffset[1] + cameraOffset[2] * cameraOffset[2];
			double distFinish2 = cameraOffset[1] * cameraOffset[1] + cameraOffset[0] * cameraOffset[0];
			distHold = sqrt(distHold);
			distHold2 = sqrt(distHold2);
			distFinish = sqrt(distFinish);
			distFinish2 = sqrt(distFinish2);
			distFinish /= distHold;
			distFinish2 /= distHold2;
			cameraOffset[1] /= ((distFinish + distFinish2) / 2);
			cameraOffset[2] /= distFinish;
			cameraOffset[0] /= distFinish2;
		}
		if (currentGame.getKeystate()[SDL_SCANCODE_KP_8] || mouseY > 0 && trackMouse || controllerRightStateY < -JOYSTICK_DEAD_ZONE) {
			//cameraOffset[1] += offsetSpeed;
			float modifier = 1;
			if(mouseY > 0){
				modifier = mouseY / 5;
			} else if(controllerRightStateY < -JOYSTICK_DEAD_ZONE){
        modifier = ((double)(controllerRightStateY * -1)) / 32767;
      }
			double distHold = cameraOffset[1] * cameraOffset[1] + cameraOffset[2] * cameraOffset[2];
			double distHold2 = cameraOffset[1] * cameraOffset[1] + cameraOffset[0] * cameraOffset[0];
			cameraOffset[1] += offsetSpeed * modifier;
			double distFinish = cameraOffset[1] * cameraOffset[1] + cameraOffset[2] * cameraOffset[2];
			double distFinish2 = cameraOffset[1] * cameraOffset[1] + cameraOffset[0] * cameraOffset[0];
			distHold = sqrt(distHold);
			distHold2 = sqrt(distHold2);
			distFinish = sqrt(distFinish);
			distFinish2 = sqrt(distFinish2);
			distFinish /= distHold;
			distFinish2 /= distHold2;
			cameraOffset[1] /= ((distFinish + distFinish2) / 2);
			cameraOffset[2] /= distFinish;
			cameraOffset[0] /= distFinish2;
		}
		if (currentGame.getKeystate()[SDL_SCANCODE_KP_4]) {
			cameraOffset[0] -= offsetSpeed;
		}
		if (currentGame.getKeystate()[SDL_SCANCODE_KP_6]) {
			cameraOffset[0] += offsetSpeed;
		}
		if (currentGame.getKeystate()[SDL_SCANCODE_KP_7] || mouseX < 0 && trackMouse || controllerRightStateX < -JOYSTICK_DEAD_ZONE) { // Rotate the camera about the y axis
			double distHold = cameraOffset[0] * cameraOffset[0] + cameraOffset[2] * cameraOffset[2];
			float multiplier = 1;
			if(mouseX < 0){
				multiplier = (mouseX * -1) / 5;
			} else if(controllerRightStateX < -JOYSTICK_DEAD_ZONE){
        multiplier = ((float) (controllerRightStateX * -1)) / 32767;
      }
			if(cameraOffset[0] <= pos[0] && cameraOffset[2] <= pos[2]){
				cameraOffset[0] += offsetSpeed * multiplier;
				cameraOffset[2] -= offsetSpeed * multiplier;
			} else if(cameraOffset[0] <= pos[0] && cameraOffset[2] >= pos[2]){
				cameraOffset[0] -= offsetSpeed * multiplier;
				cameraOffset[2] -= offsetSpeed * multiplier;
			} else if(cameraOffset[0] >= pos[0] && cameraOffset[2] <= pos[2]){
				cameraOffset[0] += offsetSpeed * multiplier;
				cameraOffset[2] += offsetSpeed * multiplier;
			} else if(cameraOffset[0] >= pos[0] && cameraOffset[2] >= pos[2]){
				cameraOffset[0] -= offsetSpeed * multiplier;
				cameraOffset[2] += offsetSpeed * multiplier;
			}
			double distFinish = cameraOffset[0] * cameraOffset[0] + cameraOffset[2] * cameraOffset[2];
			distHold = sqrt(distHold);
			distFinish = sqrt(distFinish);
			distFinish /= distHold;
			cameraOffset[0] /= distFinish;
			cameraOffset[2] /= distFinish;

		}
		if (currentGame.getKeystate()[SDL_SCANCODE_KP_9] || mouseX > 0 && trackMouse || controllerRightStateX > JOYSTICK_DEAD_ZONE) {
			double distHold = cameraOffset[0] * cameraOffset[0] + cameraOffset[2] * cameraOffset[2];
			float multiplier = 1;
			if(mouseX > 0){
				multiplier = mouseX / 5;
			} else if(controllerRightStateX > JOYSTICK_DEAD_ZONE){
        multiplier = ((float)controllerRightStateX) / 32767;
      }
			if(cameraOffset[0] <= pos[0] && cameraOffset[2] <= pos[2]){
				cameraOffset[0] -= offsetSpeed * multiplier;
				cameraOffset[2] += offsetSpeed * multiplier;
			} else if(cameraOffset[0] <= pos[0] && cameraOffset[2] >= pos[2]){
				cameraOffset[0] += offsetSpeed * multiplier;
				cameraOffset[2] += offsetSpeed * multiplier;
			} else if(cameraOffset[0] >= pos[0] && cameraOffset[2] <= pos[2]){
				cameraOffset[0] -= offsetSpeed * multiplier;
				cameraOffset[2] -= offsetSpeed * multiplier;
			} else if(cameraOffset[0] >= pos[0] && cameraOffset[2] >= pos[2]){
				cameraOffset[0] += offsetSpeed * multiplier;
				cameraOffset[2] -= offsetSpeed * multiplier;
			}
			double distFinish = cameraOffset[0] * cameraOffset[0] + cameraOffset[2] * cameraOffset[2];
			distHold = sqrt(distHold);
			distFinish = sqrt(distFinish);
			distFinish /= distHold;
			cameraOffset[0] /= distFinish;
			cameraOffset[2] /= distFinish;
		}
		if(currentGame.getKeystate()[SDL_SCANCODE_KP_MINUS]){
			cameraOffset[0] *= 1.01f; // Zoom out
			cameraOffset[1] *= 1.01f;
			cameraOffset[2] *= 1.01f;
		}
		if(currentGame.getKeystate()[SDL_SCANCODE_KP_PLUS]){
			cameraOffset[0] *= 0.99f; // Zoom in
			cameraOffset[1] *= 0.99f;
			cameraOffset[2] *= 0.99f;
		}
		if (currentGame.getKeystate()[SDL_SCANCODE_F]) {
			angles[0] -= rotateSpeed;
		}
		if (currentGame.getKeystate()[SDL_SCANCODE_R]) {
			angles[0] += rotateSpeed;
		}
		if (currentGame.getKeystate()[SDL_SCANCODE_G]) {
			angles[1] -= rotateSpeed;
		}
		if (currentGame.getKeystate()[SDL_SCANCODE_T]) {
			angles[1] += rotateSpeed;
		}
		if (currentGame.getKeystate()[SDL_SCANCODE_H]) {
			angles[2] -= rotateSpeed;
		}
		if (currentGame.getKeystate()[SDL_SCANCODE_Y]) {
			angles[2] += rotateSpeed;
		}
		if (currentGame.getKeystate()[SDL_SCANCODE_A] || controllerLeftStateX < -JOYSTICK_DEAD_ZONE) {
			double xSpeed = sin((angles[1] - 180) * (PI / 180));
			double ySpeed = cos((angles[1] - 180) * (PI / 180));

      double multiplier = 1;
      if(controllerLeftStateX < -JOYSTICK_DEAD_ZONE){
        multiplier = ((float) (controllerLeftStateX * -1)) / 32767;
      }

			pos[0] += (xSpeed / 300) * multiplier;
			pos[2] += (ySpeed / 300) * multiplier;
		}
		if (currentGame.getKeystate()[SDL_SCANCODE_D] || controllerLeftStateX > JOYSTICK_DEAD_ZONE) {
			double xSpeed = sin(angles[1] * (PI / 180));
			double ySpeed = cos(angles[1] * (PI / 180));

      double multiplier = 1;
      if(controllerLeftStateX > JOYSTICK_DEAD_ZONE){
        multiplier = ((float) (controllerLeftStateX)) / 32767;
      }

			pos[0] += (xSpeed / 300) * multiplier;
			pos[2] += (ySpeed / 300) * multiplier;
		}
		if ((currentGame.getKeystate()[SDL_SCANCODE_SPACE] || buttonCheckA) && pos[1] == 0 ){
			fallspeed = -0.003f;
			//pos[1] += 0.05f;
		}
		if (currentGame.getKeystate()[SDL_SCANCODE_E]) {
			fallspeed = 0;
			pos[1] += speed;
		}
		if (currentGame.getKeystate()[SDL_SCANCODE_Q]) {
			pos[1] -= speed;
		}
		if (currentGame.getKeystate()[SDL_SCANCODE_W] || controllerLeftStateY < -JOYSTICK_DEAD_ZONE) {
			double xSpeed = sin((angles[1] + 90) * (PI / 180));
			double ySpeed = cos((angles[1] + 90) * (PI / 180));

      double multiplier = 1;
      if(controllerLeftStateY < -JOYSTICK_DEAD_ZONE){
        multiplier = ((float) (controllerLeftStateY * -1)) / 32767;
      }

			pos[0] += (xSpeed / 300) * multiplier;
			pos[2] += (ySpeed / 300) * multiplier;
		}
		if (currentGame.getKeystate()[SDL_SCANCODE_S] || controllerLeftStateY > JOYSTICK_DEAD_ZONE) {
			double xSpeed = sin((angles[1] - 90) * (PI / 180));
			double ySpeed = cos((angles[1] - 90) * (PI / 180));

      double multiplier = 1;
      if(controllerLeftStateY > JOYSTICK_DEAD_ZONE){
        multiplier = ((float) controllerLeftStateY) / 32767;
      }

			pos[0] += (xSpeed / 300) * multiplier;
			pos[2] += (ySpeed / 300) * multiplier;
		}
		if (currentGame.getKeystate()[SDL_SCANCODE_Z]) {
			currentScale += scaleSpeed;
		}
		if (currentGame.getKeystate()[SDL_SCANCODE_X]) {
			currentScale -= scaleSpeed;
		}
		if (currentGame.getKeystate()[SDL_SCANCODE_C]) {
			currentLuminance += 0.01f;
		}
		if (currentGame.getKeystate()[SDL_SCANCODE_V]) {
			currentLuminance -= 0.01f;
		}
		if(currentGame.getKeystate()[SDL_SCANCODE_P]){
			currentGame.changeWindowMode(SDL_WINDOW_FULLSCREEN_DESKTOP);
		}
		else if(currentGame.getKeystate()[SDL_SCANCODE_O]){
			currentGame.changeWindowMode(SDL_WINDOW_FULLSCREEN);
		} else if(currentGame.getKeystate()[SDL_SCANCODE_I]){
			currentGame.changeWindowMode(0);
		}
		if(currentGame.getKeystate()[SDL_SCANCODE_U]){
			if(SDL_GetRelativeMouseMode() == SDL_FALSE){
				SDL_SetRelativeMouseMode(SDL_TRUE);
				trackMouse = 1;
				sleep(1);
			} else{
				SDL_SetRelativeMouseMode(SDL_FALSE);
				trackMouse = 0;
				sleep(1);
			}
		}

		fallspeed = basicPhysics(&pos[1], fallspeed);

		oldMouseX = mouseX;
		oldMouseY = mouseY;

		dx = mouseX - currentGame.getWidth() / 2;
		dy = mouseY - currentGame.getHeight() / 2;
		//cameraOffset[0] += dx / 10.0f;
		//cameraOffset[2] += dy / 10.0f;
		// Lock the variables we are changing to avoid conflict
		pthread_mutex_lock(infoLock);
		currentGameInfo->gameCamera->setOffset(cameraOffset);
		currentGame.setLuminance(currentLuminance);
		*currentGameInfo->angleX = angles[0];
		*currentGameInfo->angleY = angles[1];
		*currentGameInfo->angleZ = angles[2];
		*currentGameInfo->xPos = pos[0];
		*currentGameInfo->yPos = pos[1];
		*currentGameInfo->zPos = pos[2];
		pthread_mutex_unlock(infoLock);
		//printf("Mouse -> dx:%i, dy:%i\n", dx, dy);
		//printf("Pos x:%f, y:%f, z:%f. Rot x:%f, y:%f, z:%f. Scale: %f\n", *currentGameInfo->xPos, *currentGameInfo->yPos, *currentGameInfo->zPos, *currentGameInfo->angleX, *currentGameInfo->angleY, *currentGameInfo->angleZ, *currentGameInfo->scale);
		//printf("Current offset: x:%f, y:%f, z:%f\n\n", cameraOffset[0], cameraOffset[1], cameraOffset[2]);
		frameTimes[currentFrame++] = 1.0 / *currentGame.getDeltaTime();
		if (isinf(frameTimes[currentFrame-1])) {
			currentFrame--;
			frameTimes[currentFrame] = 0;
		}
		if (currentFrame >= maxFrames) {
			currentFrame = 0;
			int sumFrames = 0;
			GLfloat minimum = 0.0f;
			for (int i = 0; i < maxFrames; i++) {
				//printf("frameTimes[%i] = %f\n", i, frameTimes[i]);
				sumFrames += frameTimes[i];
				if (i == 0) {
					minimum = frameTimes[i];
				} else if (minimum > frameTimes[i]) {
					minimum = frameTimes[i];
				}
			}
			printf("AVG FPS: %f\nMIN FPS: %f\n", sumFrames / (float)maxFrames, minimum);
		}
		*currentGameInfo->updated = false;
		//printf("FPS: %f\n", 1.0/ *currentGame.getDeltaTime());
	}
  SDL_GameControllerClose(gameController1);
	pthread_exit(0);
  //exit(0);
}
