/**
 * @file game.cpp
 * @author Alec Jackson, Christian Galvez
 * @brief game.cpp and game.hpp are example files demonstrating the use of the studious
 *       game engine. These two basic game files will generate a basic scene when the
 *       engine is compiled and ran.
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#include <string>
#include <vector>
#include <iostream>
#include <game.hpp>
#ifndef GFX_EMBEDDED
#include <OpenGlGfxController.hpp>
#else
#include <OpenGlEsGfxController.hpp>
#endif
#include <AnimationController.hpp>
/*
 IMPORTANT INFORMATION FOR LOADING SHADERS/SFX:
 Currently, the below global vectors are used for loading in sound effect files,
 texture files and shaders. Adding a new sound to the soundList allows the sound
 to be played by calling the GameInstance::playSound(int soundIndex, int loop)
 method (see documentation for use). When adding a new shader to be used in the
 program, it is IMPORTANT that you pair the vertex shader with the fragment
 shader you want to use together at the same index in the fragShaders and
 vertShaders vectors. For instance, if we had a shader called swamp.vert and
 swamp.frag, we would want both shaders to occur at the same spot in the vector
 (if we already have 2 shader files present, we would add swamp.vert as the
 third element in vertShaders, and swamp.frag as the third in fragShaders). After
 doing this, you should be able to set the programId using the
 GameInstance::getProgramID(int index) method to grab the programId for your
 gameObject. If the shader is in index 2, we would call getProgramID(2) to get
 the appropriate programId. For textures, we specify a path to an image that
 will be opened for a given texture, and specify the textures to use as a
 texture pattern (where each number in the vector corresponds to the index of
 the texture to use).
*/
// Global Variables, should eventually be moved to a config file
vector<string> soundList = {
    "src/resources/sfx/music/endlessNight.wav"
};  // A list of gameSounds to load

// Lists of embedded/core shaders
#ifndef GFX_EMBEDDED
vector<string> fragShaders = {
    "src/main/shaders/core/gameObject.frag",
    "src/main/shaders/core/colliderObject.frag",
    "src/main/shaders/core/textObject.frag",
    "src/main/shaders/core/spriteObject.frag",
    "src/main/shaders/core/uiObject.frag"
};  // Contains collider renderer and basic object renderer.
vector<string> vertShaders = {
    "src/main/shaders/core/gameObject.vert",
    "src/main/shaders/core/colliderObject.vert",
    "src/main/shaders/core/textObject.vert",
    "src/main/shaders/core/spriteObject.vert",
    "src/main/shaders/core/uiObject.vert"
};  // Contains collider renderer and basic object renderer.
#else
vector<string> fragShaders = {
    "src/main/shaders/es/gameObject.frag",
    "src/main/shaders/es/colliderObject.frag",
    "src/main/shaders/es/textObject.frag"
};  // Contains collider renderer and basic object renderer.
vector<string> vertShaders = {
    "src/main/shaders/es/gameObject.vert",
    "src/main/shaders/es/colliderObject.vert",
    "src/main/shaders/es/textObject.vert"
};  // Contains collider renderer and basic object renderer.
#endif

vector<string> texturePathStage = {
    "src/resources/images/skintexture.jpg"
};
vector<string> texturePath = {
    "src/resources/images/rock_texture.jpg",
    "src/resources/images/rock_texture.jpg",
    "src/resources/images/shoetexture.jpg",
    "src/resources/images/shirttexture.jpg"
};

TextObject *fps_counter;
TextObject *collDebugText;
TextObject *pressUText;
GameObject *wolfRef, *playerRef;  // Used for collision testing
#ifdef GFX_EMBEDDED
OpenGlEsGfxController gfxController = OpenGlEsGfxController();
#else
OpenGlGfxController gfxController = OpenGlGfxController();
#endif
AnimationController animationController;

double deltaTime = 0.0f;

int runtime(GameInstance *currentGame);
int mainLoop(gameInfo *gamein);

int main(int argc, char **argv) {
    int errorNum;
    configData config;
    int flag = loadConfig(&config, "src/resources/config.txt");
    int width, height;
    if (!flag) {
        width = config.resX;
        height = config.resY;
    } else {
        width = 1280;
        height = 720;
    }
    GameInstance currentGame(soundList, vertShaders, fragShaders, &gfxController, width, height);
    currentGame.startGame(config);
    errorNum = runtime(&currentGame);
    return errorNum;
}

/*
 (int) runtime takes a (GameInstance *) gamein to create the current scene in.
 This function creates all of the GameObjects and CameraObjects in the current
 scene and creates a seperate thread for handling user input. All of the setup
 done in the runtime function is for demonstration purposes for now. The final
 studious engine product will source scene information from a .yaml file
 supplied by the user and build the game scene based on that data.

 (int) runtime returns 0 on success.
*/
int runtime(GameInstance *currentGame) {
    cout << "Building game scene!\n";
    SDL_SetRelativeMouseMode(SDL_FALSE);
    struct gameInfo currentGameInfo;
    bool isDone = false;
    cout << "Creating camera.\n";

    // Create a camera
    auto currentCamera = currentGame->createCamera(nullptr, vec3(0), 0.0f, 16.0/9.0, 4.0, 90.0);

    // Add a sprite
    auto player = currentGame->createSprite("src/resources/images/transSnorlax.png", vec3(100.0, 600.0, 0), 0.25, gfxController.getProgramId(3).get(), ObjectAnchor::BOTTOM_LEFT, "snorlax");
    
    // Add a collider object
    auto foe = currentGame->createSprite("src/resources/images/machoke.png", vec3(100.0, 600.0, 0), 0.25, gfxController.getProgramId(3).get(), ObjectAnchor::BOTTOM_LEFT, "machoke");

    player->createCollider(gfxController.getProgramId(1).get());
    foe->createCollider(gfxController.getProgramId(1).get());
    // Add objects to camera
    vector<SceneObject *> targets = {
        player,
        foe
    };

    // Add all objects to active camera
    for (auto it = targets.begin(); it != targets.end(); ++it) {
        cout << "Adding to camera: " << (*it)->getObjectName() << endl;
        currentCamera->addSceneObject(*it);
    }

    currentGameInfo.isDone = &isDone;
    currentGameInfo.gameCamera = currentCamera;
    currentGameInfo.currentGame = currentGame;
    /*
     End Scene Loading
     */
    // Additional threads should be added, pipes will most likely be required
    // Might also be a good idea to keep the parent thread local to watch for
    // unexpected failures and messages from children
    // thread rotThread(rotateShape, &currentGameInfo, playerRef);
    mainLoop(&currentGameInfo);
    isDone = true;
    // rotThread.join();
    return 0;
}

/*
 (int) mainLoop starts rendering objects in the current GameInstance to the
 main SDL window. The methods called from the currentGame object render parts
 of the scene.

 (int) mainLoop returns 0 when closed successfully. When an error occurs and the
 mainLoop closes prematurely, an error code is returned.
*/
int mainLoop(gameInfo* gamein) {
    Uint64 begin, end;
    int running = 1;
    double currentTime = 0.0, sampleTime = 1.0;
    GameInstance *currentGame = gamein->currentGame;
    int error = 0;
    vector<double> times;
    auto player = gamein->currentGame->getSceneObject("snorlax");
    while (running) {
        /// @todo Move these calls to a separate thread...
        begin = SDL_GetPerformanceCounter();
        running = currentGame->isWindowOpen();
        error = currentGame->updateObjects();
        error |= currentGame->updateWindow(); // or
        if (error) {
            return error;
        }
        // collect input
        bool upArrowState = gamein->currentGame->getKeystate()[SDL_SCANCODE_UP];
        if (upArrowState) {
            vec3 newPos = player->getPosition(vec3(0.0f, 1.0f, 0.0f));
            player->setPosition(newPos);
        }
        bool rightArrowState = gamein->currentGame->getKeystate()[SDL_SCANCODE_RIGHT];
        if (rightArrowState) {
            vec3 newPos = player->getPosition(vec3(1.0f, 0.0f, 0.0f));
            player->setPosition(newPos);
        }
        bool leftArrowState = gamein->currentGame->getKeystate()[SDL_SCANCODE_LEFT];
        if (leftArrowState) {
            vec3 newPos = player->getPosition(vec3(-1.0f, 0.0f, 0.0f));
            player->setPosition(newPos);
        }
        bool downArrowState = gamein->currentGame->getKeystate()[SDL_SCANCODE_DOWN];
        if (downArrowState) {
            vec3 newPos = player->getPosition(vec3(0.0f, -1.0f, 0.0f));
            player->setPosition(newPos);
        }
        animationController.update();
        end = SDL_GetPerformanceCounter();
        deltaTime = static_cast<double>(end - begin) / (SDL_GetPerformanceFrequency());
        currentGame->setDeltaTime(deltaTime);
        if (SHOW_FPS) {  // use sampleSize to find average FPS
            times.push_back(deltaTime);
            currentTime += deltaTime;
            if (currentTime > sampleTime) {
                currentTime = 0.0f;
                double sum = 0.0;
                vector<double>::iterator it;
                for (it = times.begin(); it != times.end(); ++it) sum += *it;
                sum /= times.size();
                times.clear();
                cout << "FPS: " << 1.0 / sum << '\n';
            }
        }
    }
    return 0;
}
