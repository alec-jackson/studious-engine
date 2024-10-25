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
    "src/resources/sfx/music/GruntyFurnace.mp3"
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

string textBoxImage =
    "src/resources/images/Banjo Textbox.png";

string sgrunty =
    "src/resources/images/Scuffed Grunty.png";

TextObject *fps_counter;
TextObject *collDebugText;
TextObject *pressUText;
GameObject *wolfRef, *playerRef;  // Used for collision testing
double deltaTime = 0.0f;
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
    GameInstance currentGame(soundList, vertShaders, fragShaders, &gfxController, width, height);
    currentGame.startGame(config);
    errorNum = runtime(&currentGame);
    return errorNum;
}

void sendMessageRoutine(UiObject *textBox, TextObject *message, gameInfo *currentGameInfo) {
    // This is essentially a cutscene, so we're going to pace each action like so
    printf("sendMessageRoutine: Entry\n");
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

    auto fpsText = currentGame->createText("FPS",
        vec3(25.0f, 670.0f, 0.0f),
        0.7f,
        "src/resources/fonts/Comic Sans MS.ttf",
        gfxController.getProgramId(2).get(),
        "fps-text");
    auto textTest = currentGame->createText("",
        vec3(250.0f, 90.0f, 0.0f),
        0.8f,
        "src/resources/fonts/Comic Sans MS.ttf",
        gfxController.getProgramId(2).get(),
        "message-text");

    auto textTest1 = currentGame->createText("",
        vec3(250.0f, 45.0f, 0.0f),
        0.8f,
        "src/resources/fonts/Comic Sans MS.ttf",
        gfxController.getProgramId(2).get(),
        "message-text2");
    auto grunty = currentGame->createSprite(sgrunty, vec3(-240.0f, 190.0f, 0.0f), 0.45f, gfxController.getProgramId(3).get(), "grunty");

    auto box = currentGame->createUi(textBoxImage, vec3(-220.0f, 150.0f, 0.0f), 0.7f, -50.0f, -50.0f, gfxController.getProgramId(4).get(), "textbox");

    KeyFrame *k0 = AnimationController::createKeyFrame(
        box->getPosition(),
        box->getStretch(),
        "",
        1.0f
    );
    KeyFrame *k1 = AnimationController::createKeyFrame(
        vec3(30.0f, 150.0f, 0.0f),  // Position
        box->getStretch(),  // Stretch
        "",
        0.3f  // Time in SECONDS
    );
    KeyFrame *k2 = AnimationController::createKeyFrame(
        vec3(30.0f, 150.0f, 0.0f),
        vec3(1035, -50, 0.0f),
        "",
        0.5f
    );

    // Keyframes for Grunty
    KeyFrame *gk0 = AnimationController::createKeyFrame(
        grunty->getPosition(),
        grunty->getPosition(),  // Ignore for sprites
        "",
        1.0f
    );

    KeyFrame *gk1 = AnimationController::createKeyFrame(
        vec3(10.0f, 190.0f, 0.0f),
        grunty->getPosition(),
        "",
        0.3f
    );

    KeyFrame *tk0 = AnimationController::createKeyFrame(
        textTest->getPosition(),
        textTest->getPosition(),
        textTest->getMessage(),
        2.0f
    );

    KeyFrame *tk1 = AnimationController::createKeyFrame(
        textTest->getPosition(),
        textTest->getPosition(),
        "Hello sweet kevin. Where is Ryan?",
        2.0f
    );

    KeyFrame *tk2 = AnimationController::createKeyFrame(
        textTest->getPosition(),
        textTest->getPosition(),
        "Hello sweet kevin. Where is Ryan?",
        2.0f
    );

    KeyFrame *tk3 = AnimationController::createKeyFrame(
        textTest->getPosition() + vec3(0.0f, 50.0f, 0.0f),
        textTest->getPosition(),
        "Hello sweet kevin. Where is Ryan?",
        2.0f
    );

    KeyFrame *t1k0 = AnimationController::createKeyFrame(
        textTest1->getPosition(),
        textTest1->getPosition(),
        textTest1->getMessage(),
        4.5f
    );

    KeyFrame *t1k1 = AnimationController::createKeyFrame(
        textTest1->getPosition(),
        textTest1->getPosition(),
        "Maybe he is playing DRG with Matty?",
        2.0f
    );

    KeyFrame *t1k2 = AnimationController::createKeyFrame(
        textTest1->getPosition() + vec3(0.0f, 50.0f, 0.0f),
        textTest1->getPosition(),
        "Maybe he is playing DRG with Matty?",
        2.0f
    );
    
    animationController.addKeyframe(box, k0);
    animationController.addKeyframe(box, k1);
    animationController.addKeyframe(box, k2);

    animationController.addKeyframe(grunty, gk0);
    animationController.addKeyframe(grunty, gk1);

    animationController.addKeyframe(textTest, tk0);
    animationController.addKeyframe(textTest, tk1);
    animationController.addKeyframe(textTest, tk2);
    animationController.addKeyframe(textTest, tk3);

    animationController.addKeyframe(textTest1, t1k0);
    animationController.addKeyframe(textTest1, t1k1);
    animationController.addKeyframe(textTest1, t1k2);

    fps_counter = fpsText;
    fps_counter->setMessage("FPS: 0");

    auto currentCamera = currentGame->createCamera(nullptr,
        vec3(5.140022f, 1.349999f, 2.309998f), 3.14159 / 5.0f, 16.0f / 9.0f, 4.0f, 90.0f);
    cout << "currentGameObject tag is NULLPTR" // playerRef->getObjectName()
        << '\n';

    // Add objects to camera
    vector<SceneObject *> targets = {
        box,
        grunty,
        fpsText,
        textTest,
        textTest1
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
    //thread rotThread(rotateShape, &currentGameInfo, playerRef);
    thread messageRoutine(sendMessageRoutine, box, textTest, &currentGameInfo);
    mainLoop(&currentGameInfo);
    isDone = true;
    //rotThread.join();
    messageRoutine.join();
    cout << "Running cleanup\n";
    currentGame->cleanup();
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
    while (running) {
        /// @todo Move these calls to a separate thread...
        begin = SDL_GetPerformanceCounter();
        running = currentGame->isWindowOpen();
        error = currentGame->updateObjects();
        error |= currentGame->updateWindow();
        if (error) {
            return error;
        }
        end = SDL_GetPerformanceCounter();
        animationController.update();
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
                fps_counter->setMessage("FPS: " + to_string(static_cast<int>(1.0 / sum)));
            }
        }
    }
    return 0;
}
