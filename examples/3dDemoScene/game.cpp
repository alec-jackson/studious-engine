/**
 * @file game.cpp
 * @author Alec Jackson, Christian Galvez
 * @brief game.cpp and game.hpp are example files demonstrating the use of the studious
 *       game engine. These two basic game files will generate a basic scene when the
 *       engine is compiled and ran.
 * @version 0.1
 * @date 2025
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <ColliderObject.hpp>
#include <GameInstance.hpp>
#include <ModelImport.hpp>
#include <SceneObject.hpp>
#include <string>
#include <vector>
#include <iostream>
#include <map>
#include "game.hpp"
#include <TextObject.hpp>
#include <OpenGlGfxController.hpp>
#include <AnimationController.hpp>
#include <FPSCameraObject.hpp>

// Global Variables, should eventually be moved to a config file
map<string, string> soundList = {
    { "bg_music", "src/resources/sfx/music/endlessNight.wav" }
};  // A list of gameSounds to load

// Lists of embedded/core shaders
#ifndef GFX_EMBEDDED
vector<ProgramData> programs = {
    { "gameObject", "src/main/shaders/core/gameObject.vert", "src/main/shaders/core/gameObject.frag" },
    { "colliderObject", "src/main/shaders/core/colliderObject.vert", "src/main/shaders/core/colliderObject.frag" },
    { "textObject", "src/main/shaders/core/textObject.vert", "src/main/shaders/core/textObject.frag" },
    { "spriteObject", "src/main/shaders/core/spriteObject.vert", "src/main/shaders/core/spriteObject.frag" },
    { "uiObject", "src/main/shaders/core/uiObject.vert", "src/main/shaders/core/uiObject.frag" }
};
#else
vector<ProgramData> programs = {
    { "gameObject", "src/main/shaders/es/gameObject.vert", "src/main/shaders/es/gameObject.frag" },
    { "colliderObject", "src/main/shaders/es/colliderObject.vert", "src/main/shaders/es/colliderObject.frag" },
    { "textObject", "src/main/shaders/es/textObject.vert", "src/main/shaders/es/textObject.frag" },
    { "spriteObject", "src/main/shaders/es/spriteObject.vert", "src/main/shaders/es/spriteObject.frag" },
    { "uiObject", "src/main/shaders/es/uiObject.vert", "src/main/shaders/es/uiObject.frag" }
};
#endif

GameInstance *currentGame;

extern std::unique_ptr<GfxController> gfxController;
extern std::unique_ptr<AnimationController> animationController;
extern std::unique_ptr<PhysicsController> physicsController;
extern std::unique_ptr<InputController> inputController;

int runtime();
int mainLoop();
void decorateAltScene();

int main() {
    int errorNum;
    auto config = StudiousConfig("src/resources/config.txt");

    GameInstance game(config);
    currentGame = &game;
    currentGame->createGameScene("3d-demo-scene");
    currentGame->createGameScene("alternate-3d-scene");

    // Load shader programs
    for (auto program : programs) {
        gfxController.get()->loadShaders(program.programName, program.vertexShaderPath, program.fragmentShaderPath);
    }
    decorateAltScene();
    currentGame->setActiveScene("3d-demo-scene");
    errorNum = runtime();
    return errorNum;
}

void decorateAltScene() {
    currentGame->setActiveScene("alternate-3d-scene");
    auto playerPoly = ModelImport::createPolygonFromFile(
        "src/resources/models/Dracula.obj");

    currentGame->createGameObject(playerPoly, vec3(0.0f, 0.0f, -1.0f),
        vec3(0.0f, 0.0f, 0.0f), 0.5f, "alt");
    currentGame->createSprite("src/resources/images/JTIconNoBackground.png", vec3(0), 0.5,
        ObjectAnchor::BOTTOM_LEFT, "altim");
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
int runtime() {
    cout << "Building game scene!\n";
    SDL_SetRelativeMouseMode(SDL_FALSE);
    cout << "Creating camera.\n";

    // Initialize sfx
    for (auto sfx : soundList) {
        currentGame->loadSound(sfx.first, sfx.second);
    }
    // Start the background music
    currentGame->playSound("bg_music", 1, 60);

    cout << "Creating Map.\n";

    auto mapPoly = ModelImport::createPolygonFromFile("src/resources/models/Forest Scene Tri.obj");

    currentGame->createGameObject(mapPoly,
        vec3(-0.006f, -0.019f, 0.0f), vec3(0.0f, 0.0f, 0.0f), 1.0f, "map");

    cout << "Creating Player\n";

    auto playerPoly = ModelImport::createPolygonFromFile(
        "src/resources/models/Dracula.obj");

    auto companionPoly = ModelImport::createPolygonFromFile(
        "src/resources/models/human.obj");

    // Ready the gameObjectInfo for the player object
    auto playerRef = currentGame->createGameObject(playerPoly, vec3(0.0f, 0.0f, -1.0f),
        vec3(0.0f, 0.0f, 0.0f), 0.5f, "player");
    playerRef->setVisible(true);
    VEC(SHD(SceneObject) companions;
    companions.push_back(currentGame->createGameObject(companionPoly, vec3(0.0f, 1.5f, 2.0f), vec3(0.0f, 270.0f, 0.0f), 0.5f, "companion"));
    companions.push_back(currentGame->createGameObject(companionPoly, vec3(0.0f, 1.5f, -2.0f), vec3(0.0f, 270.0f, 0.0f), 0.5f, "companion2"));
    companions.push_back(currentGame->createGameObject(companionPoly, vec3(2.0f, 1.5f, 0.0f), vec3(0.0f, 270.0f, 0.0f), 0.5f, "companion3"));
    companions.push_back(currentGame->createGameObject(companionPoly, vec3(-2.0f, 1.5f, 0.0f), vec3(0.0f, 270.0f, 0.0f), 0.5f, "companion4"));
    for (auto companion : companions) {
        companion->setPermanentlyVisible(true);
        playerRef->addChild(companion);
    }
    playerRef->createCollider();

    physicsController->addSceneObject(playerRef, {
        .isKinematic = true,
        .obeyGravity = true,
        .elasticity = 0.0f,
        .mass = 5.0f
    });

    cout << "Creating wolf\n";

    auto wolfPoly = ModelImport::createPolygonFromFile("src/resources/models/wolf.obj");

    auto wolfObject = currentGame->createGameObject(wolfPoly,
        vec3(-11.0f, 1.6f, 6.0f), vec3(0.0f, 0.0f, 0.0f), 1.0f, "NPC");

    // Make the wolf spin :)
    auto kf = AnimationController::createKeyFrame(
        UPDATE_ROTATION,        // Rotate
        5.0f);                 // Spin for 5 seconds

    kf->rotation.desired = vec3(0.0f, 0.0f, 720.0f);
    auto kf1 = AnimationController::createKeyFrame(
        UPDATE_ROTATION | UPDATE_POS,   // Rotate and move
        5.0f);                          // seconds

    kf1->rotation.desired = vec3(0.0f, 360.0f, 720.0f);
    kf1->pos.desired = wolfObject->getPosition() + vec3(-3.0f, 0.0f, 4.0f);
    animationController->addKeyFrame(wolfObject, kf);
    animationController->addKeyFrame(wolfObject, kf1);

    wolfObject->createCollider();

    PhysicsParams parms = {
        .isKinematic = false,
        .obeyGravity = false,
        .elasticity = 0.0f,
        .mass = 10.0f,
    };
    // add the wolf object to physics controller as non-kinematic
    physicsController->addSceneObject(wolfObject, parms);

    // Configure some in-game text objects
    currentGame->createText(
        "Studious Engine 2025",                 // Message
        vec3(25.0f, 25.0f, 0.0f),               // Position
        1.0f,                                   // Scale
        "src/resources/fonts/AovelSans.ttf",    // Font Path
        5.0f,                                   // Char spacing
        48,                                     // Font point
        0,                                      // Newline Size
        "studious-text");                       // ObjectName

    currentGame->createText(
        "Tab/Start to Focus (detached)",
        vec3(850.0f, 670.0f, 0.0f),
        0.7f,
        "src/resources/fonts/AovelSans.ttf",
        0.0f,
        48,
        0,
        "pressUText");

    auto fpsText = currentGame->createText("FPS",
        vec3(25.0f, 670.0f, 0.0f),
        0.7f,
        "src/resources/fonts/AovelSans.ttf",
        0.0f,
        48,
        0,
        "fps-text");

    currentGame->createSprite(
        "src/resources/images/JTIconNoBackground.png",
        vec3(1250.0f, 50.0f, 0.0f),
        0.1f,
        ObjectAnchor::CENTER,
        "test-sprite");

    auto bubble = currentGame->createUi(
        "src/resources/images/Message Bubble UI.png",   // image path
        vec3(80.0f, 160.0f, 0.0f),                     // Position
        0.5f,                                           // Scale
        115.0f,                                         // Width
        0.0f,                                           // Height
        ObjectAnchor::CENTER,                           // Anchor
        "uiBubble");                                    // UI Bubble
    bubble->setTint(vec4(0.4f, 0.0f, 0.0f, 0.0f));
    currentGame->createText(
        "Textbox Example",
        vec3(40.0f, 155.0f, 0.0f),
        0.6f,
        "src/resources/fonts/AovelSans.ttf",
        1.0f,
        48,
        0,
        "test-text");

    fpsText->setMessage("FPS: 0");

    vec3 fpsCameraAdditionalOffset(0.0f, 2.0f, 0.0f);
    auto offset = vec3(5.140022f, 2.349999f, 2.309998f);
    float fovDegrees = 70.0f;
    float aspectRatio = 16.0f / 9.0f;
    float farClipping = 0.01f;
    float nearClipping = 100.0f;

    currentGame->createFPSCamera(playerRef,
        offset, fpsCameraAdditionalOffset, fovDegrees, aspectRatio, farClipping, nearClipping, "fpsCamera");
    currentGame->createTPSCamera(playerRef,
        offset, fovDegrees, aspectRatio, farClipping, nearClipping, "tpsCamera");


    playerRef->setRotation(vec3(0, 0, 0));
    cout << "currentGameObject tag is " << playerRef->objectName()
        << '\n';

    playerRef->setPosition(vec3(0.0f, 0.5f, 0.0f));
    playerRef->setRotation(vec3(0.0f, 180.0f, 0.0f));
    playerRef->setScale(0.5f);

    /*
     End Scene Loading
     */
    // Additional threads should be added, pipes will most likely be required
    // Might also be a good idea to keep the parent thread local to watch for
    // unexpected failures and messages from children
    thread rotThread(rotateShape, playerRef);
    mainLoop();
    rotThread.join();
    return 0;
}

/*
 (int) mainLoop starts rendering objects in the current GameInstance to the
 main SDL window. The methods called from the currentGame object render parts
 of the scene.

 (int) mainLoop returns 0 when closed successfully. When an error occurs and the
 mainLoop closes prematurely, an error code is returned.
*/
int mainLoop() {
    double currentTime = 0.0, sampleTime = 1.0;
    int error = 0;
    vector<double> times;
    auto fpsText = currentGame->getSceneObject<TextObject>("fps-text");
    while (!currentGame->isShutDown()) {
        if (inputController->pollInput(GameInput::QUIT)) currentGame->shutdown();
        error = currentGame->update();
        if (error) {
            return error;
        }

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
                fpsText->setMessage("FPS: " + to_string(static_cast<int>(1.0 / sum)));
            }
        }
    }
    return 0;
}
