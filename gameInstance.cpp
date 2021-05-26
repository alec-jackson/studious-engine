#include "gameInstance.hpp"

/*
(void) startGameInstance uses the passed struct (gameInstanceArgs) args to
configure the new GameInstance. The args struct is defined as the following:
int windowWidth - Defines the width of the application window
int windowHeight - Defines the height of the application window
vector<string> soundList - Contains paths to sound effects to load
vector<string> vertexShaders - Contains paths to vertex shaders
vector<string> fragmentShaders - Contains paths to vertex shaders
pthread_mutex_t &lock - Reference to lock used for locking all shared resources

(void) startGameInstance does not return any value.
*/
void GameInstance::startGameInstance(gameInstanceArgs args) {
	infoLock = args.lock;
	sfxNames = args.soundList;
	width = args.windowWidth;
	height = args.windowHeight;
	luminance = 1.0f; // Set default values
	directionalLight = vec3(-100, 100, 100);
	controllersConnected = 0;
	initWindow(width, height);
	initAudio();
	playSound(0, 1);
	initController();
	initApplication(args.vertexShaders, args.fragmentShaders);
	keystate = SDL_GetKeyboardState(NULL);
	gameCameraCount = 0;
	gameObjectCount = 0;
	gameObjects = NULL;
	gameCameras = NULL;
	text.initText();
}

/*
(int) getWidth returns the current width of the single window for the current
GameInstance.
*/
int GameInstance::getWidth() {
	return width;
}

/*
(int) getHeight returns the current height of the single window for the current
GameInstance.
*/
int GameInstance::getHeight() {
	return height;
}

/*
(vec3) getDirectionalLight returns the current location of the directionalLight
in the scene.
*/
vec3 GameInstance::getDirectionalLight() {
	return directionalLight;
}

/*
(const Uint8 *) getKeystate returns the current keystate of the current
GameInstance. The keystate is used for getting input from the user's keyboard.
*/
const Uint8 * GameInstance::getKeystate() {
	return keystate;
}

/*
(GLuint) getProgramID returns the programID associated with the given
(int) index. On success, function returns associated programID. On failure,
-1 is returned and an error is written to stderr.
*/
GLuint GameInstance::getProgramID(int index) {
	if (index < 0 || index >= programID.size()) {
		cerr << "Error: Requested programID is not in available range [0, "
			<< programID.size() - 1 << "]\n";
	}
	return programID[index];
}
controllerReadout* GameInstance::getControllers(int controllerIndex){
	controllerInfo[controllerIndex].leftAxis = SDL_GameControllerGetAxis(gameControllers[controllerIndex], SDL_CONTROLLER_AXIS_LEFTY );
	return &controllerInfo[controllerIndex];
}
int GameInstance::getControllersConnected() {
	return controllersConnected;
}
void GameInstance::swapBuffers() {
	SDL_GL_SwapWindow(window);
}
void GameInstance::playSound(int soundIndex, int loop) {
	Mix_PlayChannel(-1, sound[soundIndex], loop);
}
void GameInstance::changeWindowMode(int mode){
	SDL_DisplayMode DM;
	// if(mode > 1){
	// 	SDL_GetCurrentDisplayMode(0, &DM);
	// } else {
	// 	DM.w = width;
	// 	DM.h = height;
	// }
	//SDL_SetWindowSize(window, DM.w, DM.h);
	SDL_SetWindowFullscreen(window, mode);
	//SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}
void GameInstance::cleanup() {
	for (int i = 0; i < controllersConnected; i++) {
		//SDL_GameControllerClose(gameControllers[i]);
	}
	vector<GLuint>::iterator it;
	for (it = programID.begin(); it != programID.end(); ++it) {
		glDeleteProgram(*it);
	}
	gameObjectLL *currentGameObject = gameObjects;
	gameObjectLL *temp;
	for (int i = 0; i < gameObjectCount; i++) {
		temp = currentGameObject;
		currentGameObject = currentGameObject->next;
		destroyGameObject(temp->current);
		free(temp);
	}
	glDeleteVertexArrays(1, &vertexArrayID);
	Mix_CloseAudio();
	SDL_DestroyWindow(window);
	SDL_Quit();
	return;
}
/*
Function used for properly destroying gameobjects in the game
 instance. Function returns a 0 after a successful clean. If the
 function returns a -1, the function was passed a null pointer.
*/
int GameInstance::destroyGameObject(GameObject *object) {
	if (object == NULL) {
		return -1;
	}

	// Delete OpenGL buffers for the gameObjects
	for (int i = 0; i < object->getModel()->numberOfObjects; i++) {
		glDeleteBuffers(1, &object->getModel()->shapebufferID[i]);
		glDeleteBuffers(1, &object->getModel()->textureCoordsID[i]);
		glDeleteBuffers(1, &object->getModel()->normalbufferID[i]);
		glDeleteTextures(1, &object->getModel()->textureID[i]);
		free(object->getModel()->vertices[i]);
		free(object->getModel()->normalCoords[i]);
		if (object->getModel()->textureCoords[i] != NULL) {
			free(object->getModel()->textureCoords[i]);
		}
	}
	free(object->getModel()->textureCoords);
	free(object->getModel()->vertices);
	free(object->getModel()->normalCoords);
	free(object->getModel()->shapebufferID);
	free(object->getModel()->normalbufferID);
	free(object->getModel()->textureCoordsID);
	free(object->getModel()->textureID);
	free(object->getModel()->pointCount);
	free(object->getModel());
	free(object);
	return 0;
}

// Go through gameObjectLL and call drawShape method on each.
void GameInstance::mainLoop(bool *updated) {
	clock_t begin, end;
	int running = 1;
	while (running) {
		while (*updated && running) {
			//printf("HALTING DRAW\n");
		}
		while(SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || keystate[SDL_SCANCODE_ESCAPE]) {
				printf("Closing now...\n");
				running = 0;
			}
		}
		begin = clock();
		//printf("Keystate is currently %ul\n", *keystate);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glClearColor(0.0f, 0.0f, 0.0, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glUseProgram(programID[0]);
		gameCameraLL *currentGameCamera = gameCameras;
		if (currentGameCamera == NULL) {
			fprintf(stderr, "No cameras found in active scene!\n");
			break;
		}
		// Update the VP matrix for each camera
		for (int i = 0; i < gameCameraCount; i++) {
			//printf("Currently updating camera %i\n", i);
			//printf("gameCameraCount = %i\n", gameCameraCount);
			currentGameCamera->current->updateCamera();
			//printf("Camera update successful!\n");
		}
		//printf("%d\n", gameObjectCount);
		//vec3 color = vec3(1,1,1);
		//text.drawText("testing", 0, 0, 1, color);
		gameObjectLL *currentGameObject = gameObjects;
		for (int i = 0; i < gameObjectCount; i++) {
			//printf("Drawing GameObject %i\n", i);
			currentGameObject->current->setLock(infoLock);
			currentGameObject->current->setDirectionalLight(directionalLight);
			currentGameObject->current->setLuminance(luminance);
			// Send the new VP matrix to the current gameObject being drawn.
			currentGameObject->current->setVPMatrix(getCamera(currentGameObject->current->getCameraID())->getVPMatrix());
			//printf("Successfully set VP Matrix for GameObject %i\n", i);
			currentGameObject->current->drawShape();
			currentGameObject = currentGameObject->next;
			//printf("Currently drawing shape %i\n", i);
		}
		SDL_GL_SwapWindow(window);
		end = clock();
		deltaTime = (double)(end - begin) / (double)CLOCKS_PER_SEC;
		//printf("FPS: %f\n", 1 / deltaTime);
		*updated = true;
	}
	printf("Running cleanup\n");
	cleanup();
}

int GameInstance::isWindowClosed(){
	short running = 1;
	while(SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT || keystate[SDL_SCANCODE_ESCAPE]) {
			printf("Closing now...\n");
			running = 0;
		}
	}
	return running;
}

void GameInstance::updateOGL(){
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearColor(0.0f, 0.0f, 0.0, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

int GameInstance::updateCameras(){
	gameCameraLL *currentGameCamera = gameCameras;
	if (currentGameCamera == NULL) {
		fprintf(stderr, "No cameras found in active scene!\n");
		return 1;
	}
	// Update the VP matrix for each camera
	for (int i = 0; i < gameCameraCount; i++) {
		currentGameCamera->current->updateCamera();
	}
	return 0;
}

int GameInstance::updateObjects(){
	gameObjectLL *currentGameObject = gameObjects;
	for (int i = 0; i < gameObjectCount; i++) {
		currentGameObject->current->setLock(infoLock);
		currentGameObject->current->setDirectionalLight(directionalLight);
		currentGameObject->current->setLuminance(luminance);
		// Send the new VP matrix to the current gameObject being drawn.
		currentGameObject->current->setVPMatrix(getCamera(currentGameObject->current->getCameraID())->getVPMatrix());
		currentGameObject->current->drawShape();
		currentGameObject = currentGameObject->next;
	}
	return 0;
}

int GameInstance::updateWindow(){
	SDL_GL_SwapWindow(window);
	return 0;
}

int GameInstance::setDeltaTime(GLdouble time){
	deltaTime = time;
	return 0;
}

// Creates a gameobject and returns the index of where it sits in the
// gameObjects array.
int GameInstance::createGameObject(gameObjectInfo objectInfo) {
	printf("Creating gameobject %i\n", gameObjectCount);
	// If the gameObjects array is empty
	if (gameObjects == NULL) {
		printf("Creating gameObject 0\n");
		gameObjects = (gameObjectLL*)calloc(1, sizeof(gameObjectLL));
		if (gameObjects == NULL) {
			fprintf(stderr, "Failed to allocate memory for new Game Object!");
			return -1;
		}
		gameObjects->current = (GameObject*)calloc(1, sizeof(GameObject));
		if (gameObjects->current == NULL) {
			fprintf(stderr, "Failed to allocate memory for new Game Object!");
			return -1;
		}
		gameObjects->current->configureGameObject(objectInfo);
		gameObjectCount = 1;
		gameObjects->next = NULL; // Needed for weird macOS calloc() behavior.
		return 0;
	} else {
		//printf("Creating gameObject %i\n", gameObjectCount);
		int currentIndex = 1;
		gameObjectLL *currentGameObject = gameObjects;
		while (currentGameObject->next != NULL) {
			currentGameObject = currentGameObject->next;
			currentIndex++;
		}
		printf("Loop safe\n");
		currentGameObject->next = (gameObjectLL*)malloc(sizeof(gameObjectLL));
		currentGameObject = currentGameObject->next;
		if (currentGameObject == NULL) {
			fprintf(stderr, "Failed to allocate memory for new Game Object!");
			return -1;
		}
		currentGameObject->current = (GameObject*)malloc(sizeof(GameObject));
		if (currentGameObject->current == NULL) {
			fprintf(stderr, "Failed to allocate memory for new Game Object!");
			return -1;
		}
		currentGameObject->current->configureGameObject(objectInfo);
		gameObjectCount++;
		currentGameObject->next = NULL;
		return currentIndex;
	}
}

// Create the gameCamera for the scene. Assume there is a camera 0 when
// starting up the main loop.
int GameInstance::createCamera(cameraInfo camInfo) {
	// If the gameObjects array is empty
	if (gameCameras == NULL) {
		printf("Creating GameCamera 0\n");
		gameCameras = (gameCameraLL*)calloc(1, sizeof(gameCameraLL));
		if (gameCameras == NULL) {
			fprintf(stderr, "Failed to allocate memory for new Game Object!");
			return -1;
		}
		gameCameras->current = (GameCamera*)calloc(1, sizeof(GameCamera));
		if (gameCameras->current == NULL) {
			fprintf(stderr, "Failed to allocate memory for new Game Object!");
			return -1;
		}
		gameCameras->current->configureCamera(camInfo);
		gameCameraCount = 1;
		gameCameras->next = NULL; // Needed for weird macOS calloc() behavior.
		return 0;
	} else {
		printf("Creating GameCamera %i\n", gameCameraCount);
		int currentIndex = 1;
		gameCameraLL *currentGameCamera = gameCameras;
		while (currentGameCamera->next != NULL) {
			currentGameCamera = currentGameCamera->next;
			currentIndex++;
		}
		printf("Loop safe\n");
		currentGameCamera->next = (gameCameraLL*)malloc(sizeof(gameCameraLL));
		currentGameCamera = currentGameCamera->next;
		if (currentGameCamera == NULL) {
			fprintf(stderr, "Failed to allocate memory for new Game Object!");
			return -1;
		}
		currentGameCamera->current = (GameCamera*)malloc(sizeof(GameCamera));
		if (currentGameCamera->current == NULL) {
			fprintf(stderr, "Failed to allocate memory for new Game Object!");
			return -1;
		}
		currentGameCamera->current->configureCamera(camInfo);
		gameCameraCount++;
		currentGameCamera->next = NULL;
		return currentIndex;
	}
}

/*
 getGameObject returns a GameObject with the matching gameObjectID in the
 current context.
 */
GameObject * GameInstance::getGameObject(int gameObjectID) {
	if (gameObjectID > gameObjectCount - 1) {
		printf("GameObject with GameObject ID %i does not exist in the current context!\n", gameObjectID);
		return NULL;
	}
	gameObjectLL *currentGameObject = gameObjects;
	for (int i = 0; i < gameObjectID; i++) {
		currentGameObject = currentGameObject->next;
	}
	return currentGameObject->current;
}

GameCamera * GameInstance::getCamera(int gameCameraID) {
	//printf("Currently fetching gameCamera %i.\n", gameCameraID);
	if (gameCameraID > gameCameraCount - 1) {
		printf("GameCamera with GameCamera ID %i does not exist in the current context!\n", gameCameraID);
		return NULL;
	}
	gameCameraLL *currentGameCamera = gameCameras;
	for (int i = 0; i < gameCameraID; i++) {
		currentGameCamera = currentGameCamera->next;
	}
	//printf("Returning gameCamera %i\n", gameCameraID);
	return currentGameCamera->current;
}

GLdouble * GameInstance::getDeltaTime() {
	return &deltaTime;
}

void GameInstance::setLuminance(GLfloat luminanceValue) {
	luminance = luminanceValue;
}


void GameInstance::initWindow(int width, int height) {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
	window = SDL_CreateWindow("OGL Engine Beta", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
	//SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI, &window, &renderer);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GLContext mainContext = SDL_GL_CreateContext(window);
	renderer = SDL_GetRenderer(window);
	//SDL_RenderSetLogicalSize(renderer, 270, 480);

	if (window == NULL) {
		fprintf(stderr, "Failed to create SDL window!");
		return;
	}
	if (glewInit() != GLEW_OK) {
		printf("Failed to initialize GLEW!\n");
		return;
	}
}

void GameInstance::initAudio() {
	audioID = Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 512);
	if (audioID < 0) {
		fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
		exit(-1);
	}
	audioID = Mix_AllocateChannels(4);
	if (audioID < 0) {
		fprintf(stderr, "Unable to allocate mixing channels: %s\n", SDL_GetError());
		exit(-1);
	}
	vector<string>::iterator it;
	int i = 0;
	for (it = sfxNames.begin(); it != sfxNames.end(); ++it) {
		sound.push_back(Mix_LoadWAV((*it).c_str()));
		if (sound[i++] == NULL) {
			cerr << "Unable to load wave file: " << sfxNames[i] << '\n';
		}
	}
}

void GameInstance::initController(){
	int joyFlag = SDL_NumJoysticks();
	printf("Number of Joysticks Connected:%d\n", joyFlag);
	if(joyFlag < 1){
		printf("Warning: No JoySticks Detected\n");
	} else {
		for(int i = 0; i < joyFlag; i++){
			if(SDL_IsGameController(i)){
				gameControllers[controllersConnected] = SDL_GameControllerOpen(i);
				if(gameControllers[controllersConnected] == NULL){
					printf("Warning: Unable to open game controller: %s\n", SDL_GetError());
				} else {
					controllersConnected++;
					return;
				}
			}
		}
		printf("No avaible Joysick is an Xinput style gamepad\n");
	}
	return;
}

void GameInstance::initApplication(vector<string> vertexPath, vector<string> fragmentPath) {
	// Compile each of our shaders and assign them their own programID number
	GLuint vertexArrayID;
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);
	for (int i = 0; i < vertexPath.size(); i++) {
		programID.push_back(LoadShaders(vertexPath[i].c_str(), fragmentPath[i].c_str()));
	}
}

void GameInstance::basicCollision(GameInstance* gameInstance) {
	gameObjectLL* objList = gameInstance -> gameObjects;
	GameObject* curObj;
	int runFlag = 1;

	if(objList -> current != 0){
		do{
			curObj = objList -> current;


		}while(runFlag);
	}
}
