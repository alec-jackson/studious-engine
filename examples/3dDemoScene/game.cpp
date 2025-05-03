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
#include <map>
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
map<string, string> soundList = {
    { "bg_music", "src/resources/sfx/music/endlessNight.wav" }
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
    "src/main/shaders/es/textObject.frag",
    "src/main/shaders/es/spriteObject.frag",
    "src/main/shaders/es/uiObject.frag"
};  // Contains collider renderer and basic object renderer.
vector<string> vertShaders = {
    "src/main/shaders/es/gameObject.vert",
    "src/main/shaders/es/colliderObject.vert",
    "src/main/shaders/es/textObject.vert",
    "src/main/shaders/es/spriteObject.vert",
    "src/main/shaders/es/uiObject.vert"
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
    GameInstance currentGame(vertShaders, fragShaders, &gfxController, &animationController, width, height);
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

    // Initialize sfx
    for (auto sfx : soundList) {
        currentGame->loadSound(sfx.first, sfx.second);
    }
    // Start the background music
    currentGame->playSound("bg_music", 1, 60);

    /// @todo Make loading textures for objects a little more user friendly
    // The patterns below refer to which texture to use in the texturePath, 0 meaning the first path in the array
    vector<int> texturePattern = {0, 1, 2, 3};
    vector<int> texturePatternStage = {0, 0, 0, 0};

    cout << "Creating Map.\n";

    auto mapPoly = ModelImport("src/resources/models/map3.obj",
        texturePathStage,
        texturePatternStage,
        gfxController.getProgramId(0).get())
        .createPolygonFromFile();

    auto mapObject = currentGame->createGameObject(&mapPoly,
        vec3(-0.006f, -0.019f, 0.0f), vec3(0.0f, 0.0f, 0.0f), 0.009500f, "map");

    cout << "Creating Player\n";

    auto playerPoly = ModelImport(
        "src/resources/models/Dracula.obj",
        texturePath,
        texturePattern,
        gfxController.getProgramId(0).get())
        .createPolygonFromFile();

    // Ready the gameObjectInfo for the player object
    playerRef = currentGame->createGameObject(&playerPoly, vec3(0.0f, 0.0f, -1.0f),
        vec3(0.0f, 0.0f, 0.0f), 0.005f, "player");
    playerRef->createCollider(gfxController.getProgramId(1).get());

    cout << "Creating wolf\n";

    auto wolfPoly = ModelImport("src/resources/models/wolf.obj",
        texturePath,
        texturePattern,
        gfxController.getProgramId(0).get())
        .createPolygonFromFile();

    auto wolfObject = currentGame->createGameObject(&wolfPoly,
        vec3(0.00f, 0.01f, -0.08f), vec3(0.0f, 0.0f, 0.0f), 0.02f, "NPC");

    // Make the wolf spin :)
    auto kf = AnimationController::createKeyFrame(
        UPDATE_ROTATION,        // Rotate
        5.0f);                 // Spin for 5 seconds

    kf->rotation.desired = vec3(0.0f, 0.0f, 720.0f);
    auto kf1 = AnimationController::createKeyFrame(
        UPDATE_ROTATION | UPDATE_POS,   // Rotate and move
        5.0f);                          // seconds

    kf1->rotation.desired = vec3(0.0f, 360.0f, 720.0f);
    kf1->pos.desired = wolfObject->getPosition() + vec3(0.07f, 0.0f, 0.05f);
    animationController.addKeyFrame(wolfObject, kf);
    animationController.addKeyFrame(wolfObject, kf1);

    wolfObject->createCollider(gfxController.getProgramId(1).get());
    wolfRef = wolfObject;

    // Configure some in-game text objects
    auto engineText = currentGame->createText(
        "Studious Engine 2025",                 // Message
        vec3(25.0f, 25.0f, 0.0f),               // Position
        1.0f,                                   // Scale
        "src/resources/fonts/AovelSans.ttf",    // Font Path
        5.0f,                                   // Char spacing
        48,
        gfxController.getProgramId(2).get(),    // ProgramId
        "studious-text");                       // ObjectName

    auto contactText = currentGame->createText(
        "Contact",                              // Message
        vec3(25.0f, 300.0f, 0.0f),              // Position
        0.7f,                                   // Scale
        "src/resources/fonts/AovelSans.ttf",    // Font Path
        0.0f,                                   // Char spacing
        48,
        gfxController.getProgramId(2).get(),    // ProgramId
        "contact-text");                        // ObjectName

    pressUText = currentGame->createText(
        "Press 'U' to attach/detach mouse",
        vec3(800.0f, 670.0f, 0.0f),
        0.7f,
        "src/resources/fonts/AovelSans.ttf",
        0.0f,
        48,
        gfxController.getProgramId(2).get(),
        "contact-text");

    collDebugText = contactText;
    collDebugText->setMessage("Contact: False");

    auto fpsText = currentGame->createText("FPS",
        vec3(25.0f, 670.0f, 0.0f),
        0.7f,
        "src/resources/fonts/AovelSans.ttf",
        0.0f,
        48,
        gfxController.getProgramId(2).get(),
        "fps-text");

    auto testSprite = currentGame->createSprite(
        "src/resources/images/JTIconNoBackground.png",
        vec3(1250.0f, 50.0f, 0.0f),
        0.1f,
        gfxController.getProgramId(3).get(),
        ObjectAnchor::CENTER,
        "test-sprite");

    auto testUi = currentGame->createUi(
        "src/resources/images/Message Bubble UI.png",   // image path
        vec3(80.0f, 160.0f, 0.0f),                     // Position
        0.5f,                                           // Scale
        115.0f,                                         // Width
        0.0f,                                           // Height
        gfxController.getProgramId(4).get(),            // Shader pair
        ObjectAnchor::CENTER,                           // Anchor
        "uiBubble");                                    // UI Bubble
    auto testText = currentGame->createText(
        "Textbox Example",
        vec3(40.0f, 155.0f, 0.0f),
        0.6f,
        "src/resources/fonts/AovelSans.ttf",
        1.0f,
        48,
        gfxController.getProgramId(2).get(),
        "test-text");

    fps_counter = fpsText;
    fps_counter->setMessage("FPS: 0");

    auto currentCamera = currentGame->createCamera(playerRef,
        vec3(5.140022f, 1.349999f, 2.309998f), 3.14159 / 5.0f, 16.0f / 9.0f, 4.0f, 90.0f);
    playerRef->setRotation(vec3(0, 0, 0));
    cout << "currentGameObject tag is " << playerRef->getObjectName()
        << '\n';

    playerRef->setPosition(vec3(-0.005f, 0.01f, 0.0f));
    playerRef->setRotation(vec3(0.0f, 180.0f, 0.0f));
    playerRef->setScale(0.0062f);

    // Add objects to camera
    vector<SceneObject *> targets = {
        mapObject,
        playerRef,
        wolfObject,
        engineText,
        contactText,
        fpsText,
        pressUText,
        testSprite,
        testUi,
        testText
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
    thread rotThread(rotateShape, &currentGameInfo, playerRef);
    mainLoop(&currentGameInfo);
    isDone = true;
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
int mainLoop(gameInfo* gamein) {
    Uint64 begin, end;
    int running = 1, collision = 0;
    double currentTime = 0.0, sampleTime = 1.0;
    GameInstance *currentGame = gamein->currentGame;
    int error = 0;
    vector<double> times;
    while (running) {
        /// @todo Move these calls to a separate thread...
        begin = SDL_GetPerformanceCounter();
        running = currentGame->isWindowOpen();
        error = currentGame->updateObjects();
        error |= currentGame->updateWindow();
        if (error) {
            return error;
        }
        collision = currentGame->getCollision(playerRef, wolfRef, vec3(0, 0, 0));
        string collMessage;
        if (collision == 1) {
            collMessage = "Contact: True";
        } else {
            collMessage = "Contact: False";
        }
        collDebugText->setMessage(collMessage);
        animationController.update();
        end = SDL_GetPerformanceCounter();
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
                fps_counter->setMessage("FPS: " + to_string(static_cast<int>(1.0 / sum)));
            }
        }
    }
    return 0;
}
