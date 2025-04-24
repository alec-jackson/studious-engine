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
#include <cstdio>
#ifndef GFX_EMBEDDED
#include <OpenGlGfxController.hpp>
#else
#include <OpenGlEsGfxController.hpp>
#endif
#include <AnimationController.hpp>
#include <GameInstance.hpp>

// Lists of embedded/core shaders
#ifndef GFX_EMBEDDED
vector<string> fragShaders = {
    "shaders/core/gameObject.frag",
    "shaders/core/colliderObject.frag",
    "shaders/core/textObject.frag",
    "shaders/core/spriteObject.frag",
    "shaders/core/uiObject.frag"
};  // Contains collider renderer and basic object renderer.
vector<string> vertShaders = {
    "shaders/core/gameObject.vert",
    "shaders/core/colliderObject.vert",
    "shaders/core/textObject.vert",
    "shaders/core/spriteObject.vert",
    "shaders/core/uiObject.vert"
};  // Contains collider renderer and basic object renderer.
#else
vector<string> fragShaders = {
    "shaders/es/gameObject.frag",
    "shaders/es/colliderObject.frag",
    "shaders/es/textObject.frag",
    "shaders/es/spriteObject.frag",
    "shaders/es/uiObject.frag"
};  // Contains collider renderer and basic object renderer.
vector<string> vertShaders = {
    "shaders/es/gameObject.vert",
    "shaders/es/colliderObject.vert",
    "shaders/es/textObject.vert",
    "shaders/es/spriteObject.vert",
    "shaders/es/uiObject.vert"
};  // Contains collider renderer and basic object renderer.
#endif

#ifdef GFX_EMBEDDED
OpenGlEsGfxController gfxController = OpenGlEsGfxController();
#else
OpenGlGfxController gfxController = OpenGlGfxController();
#endif
AnimationController animationController;

int runtime(GameInstance *currentGame);
int mainLoop(GameInstance *currentGame, CameraObject *currentCamera);

int main(int argc, char **argv) {
    int errorNum;
    configData config;
    int flag = loadConfig(&config, "config.txt");
    int width, height;
    if (!flag) {
        width = config.resX;
        height = config.resY;
    } else {
        width = 1280;
        height = 720;
    }
    GameInstance currentGame(vertShaders, fragShaders, &gfxController, width, height);
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
    cout << "Creating camera.\n";

    auto currentCamera = currentGame->createCamera(nullptr, vec3(0), 0.0, 16.0 / 9.0, 4.0, 90.0);
    /*
     End Scene Loading
     */
    mainLoop(currentGame, currentCamera);
    return 0;
}

/*
 (int) mainLoop starts rendering objects in the current GameInstance to the
 main SDL window. The methods called from the currentGame object render parts
 of the scene.

 (int) mainLoop returns 0 when closed successfully. When an error occurs and the
 mainLoop closes prematurely, an error code is returned.
*/
int mainLoop(GameInstance *currentGame, CameraObject *currentCamera) {
    Uint64 begin, end;
    int running = 1;
    double currentTime = 0.0, sampleTime = 1.0;
    int error = 0;
    vector<double> times;
    while (running) {
        begin = SDL_GetPerformanceCounter();
        running = currentGame->isWindowOpen();
        error = currentGame->updateObjects();
        error |= currentGame->updateWindow();
        if (error) {
            return error;
        }
        animationController.update();
        end = SDL_GetPerformanceCounter();
        // Update player position
        deltaTime = static_cast<double>(end - begin) / (SDL_GetPerformanceFrequency());
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
