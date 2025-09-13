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
#include "GameInstance.hpp"
#include <physics.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <cstdio>
#include <OpenGlGfxController.hpp>
#include <AnimationController.hpp>

// Lists of embedded/core shaders
#ifndef GFX_EMBEDDED
vector<ProgramData> programs = {
    { "gameObject", "src/main/shaders/core/gameObject.vert", "src/main/shaders/core/gameObject.frag" },
    { "colliderObject", "src/main/shaders/core/colliderObject.vert", "src/main/shaders/core/colliderObject.frag" },
    { "textObject", "src/main/shaders/core/textObject.vert", "src/main/shaders/core/textObject.frag" },
    { "spriteObject", "src/main/shaders/core/spriteObject.vert", "src/main/shaders/core/spriteObject.frag" },
    { "uiObject", "src/main/shaders/core/uiObject.vert", "src/main/shaders/core/uiObject.frag" },
    { "tileObject", "src/main/shaders/core/tileObject.vert", "src/main/shaders/core/tileObject.frag" }
};
#else
vector<ProgramData> programs = {
    { "gameObject", "src/main/shaders/es/gameObject.vert", "src/main/shaders/es/gameObject.frag" },
    { "colliderObject", "src/main/shaders/es/colliderObject.vert", "src/main/shaders/es/colliderObject.frag" },
    { "textObject", "src/main/shaders/es/textObject.vert", "src/main/shaders/es/textObject.frag" },
    { "spriteObject", "src/main/shaders/es/spriteObject.vert", "src/main/shaders/es/spriteObject.frag" },
    { "uiObject", "src/main/shaders/es/uiObject.vert", "src/main/shaders/es/uiObject.frag" },
    { "tileObject", "src/main/shaders/es/tileObject.vert", "src/main/shaders/es/tileObject.frag" }
};
#endif

extern std::unique_ptr<GfxController> gfxController;
extern std::unique_ptr<AnimationController> animationController;
extern std::unique_ptr<PhysicsController> physicsController;

int runtime(GameInstance *currentGame);
int mainLoop(GameInstance *currentGame);

int main() {
    int errorNum;
    auto config = StudiousConfig("src/resources/config.txt");

    GameInstance currentGame(config);

    // Load shader programs
    for (auto program : programs) {
        gfxController.get()->loadShaders(program.programName, program.vertexShaderPath, program.fragmentShaderPath);
    }
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

    currentGame->createCamera(nullptr, vec3(0), 0.0, 16.0 / 9.0, 4.0, 90.0, "mainCamera");
    auto player = currentGame->createSprite("src/resources/images/JTIconNoBackground.png", vec3(0), 0.5,
        ObjectAnchor::BOTTOM_LEFT, "player");
    // Attach some other objects to the parent...
    auto playerAccessory = currentGame->createSprite("src/resources/images/rockwall.jpg", vec3(0), -0.4, ObjectAnchor::BOTTOM_LEFT, "playerAcc1");
    auto playerAccessoryToo = currentGame->createSprite("src/resources/images/rockwall.jpg", vec3(100, 100, 0), -0.4, ObjectAnchor::BOTTOM_LEFT, "playerAcc1");

    playerAccessory->setParent(player);
    playerAccessoryToo->setParent(player);
    player->createCollider();

    auto obstacle = currentGame->createSprite("src/resources/images/dot_image.png",
        vec3(300, 500, 0), 10, ObjectAnchor::CENTER, "obstacle");

    obstacle->createAnimation(5, 4, 24);
    obstacle->createCollider();

    /* Create an animation track for the obstacle */
    vector<int> animationTrack = {
        0, 1, 2, 3
    };
    animationController.get()->addTrack(
        obstacle,
        "one to four",
        animationTrack,
        1,
        true);
    animationController.get()->addTrack(
        obstacle,
        "all frames",
        {},
        12,
        true);
    animationController.get()->playTrack("all frames");

    // Create a new tile object and add it to the scene
    currentGame->createTileMap(
        {{ "floor_0", "src/resources/images/rock_texture.jpg" }},
        {{ 0, 0, "floor_0" },
        { 0, 1, "floor_0" },
        { 1, 1, "floor_0" },
        { -1, -1, "floor_0" }},
        vec3(200, 200, 0),
        0.1f,
        ObjectAnchor::BOTTOM_LEFT,
        "test-tile");

    /*
     End Scene Loading
     */
    mainLoop(currentGame);
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
int mainLoop(GameInstance *currentGame) {
    Uint64 begin, end;
    double currentTime = 0.0, sampleTime = 1.0;
    int error = 0;
    vector<double> times;
    auto playerPtr = reinterpret_cast<GameObject2D *>(currentGame->getSceneObject("player"));
    auto obstaclePtr = reinterpret_cast<GameObject2D *>(currentGame->getSceneObject("obstacle"));
    float speed = 5.0f;
    vec3 offset;
    vec3 newPos;
    bool eDown = false;
    while (!currentGame->isShutDown()) {
        offset = vec3(0);
        /// @todo Move these calls to a separate thread...
        begin = SDL_GetPerformanceCounter();
        if (currentGame->pollInput(GameInput::QUIT)) currentGame->shutdown();
        error = currentGame->update();
        if (error) {
            return error;
        }
        end = SDL_GetPerformanceCounter();
        if (currentGame->pollInput(GameInput::NORTH)) offset += vec3(0, speed, 0);
        if (currentGame->pollInput(GameInput::SOUTH)) offset -= vec3(0, speed, 0);
        if (currentGame->pollInput(GameInput::EAST)) offset += vec3(speed, 0, 0);
        if (currentGame->pollInput(GameInput::WEST)) offset -= vec3(speed, 0, 0);
        if (currentGame->pollInput(GameInput::X) && !eDown) {
            printf("E pressed!\n");
            eDown = true;
            animationController.get()->pauseTrack("obstacle");
        } else if (!currentGame->pollInput(GameInput::X) && eDown) {
            printf("E released!\n");
            eDown = false;
            animationController.get()->playTrack("one to four");
        }
        newPos = playerPtr->getPosition(offset);
        playerPtr->setPosition(newPos);
        if (currentGame->getCollision2D(playerPtr, obstaclePtr, vec3(0))) printf("CONTACT TRUE\n");
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
