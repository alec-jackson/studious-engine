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
int WORDS_PER_LINE = 8; // Arbitrary
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


int showMessage(string message, CameraObject *renderer, GameInstance *currentGame) {
    auto topLine = vec3(250.0f, 90.0f, 0.0f);
    auto bottomLine = vec3(250.0f, 45.0f, 0.0f);
    auto shift = vec3(0.0f, 50.0f, 0.0f); // Text transform when "wiping"
    auto textScale = 0.8f; // Good for 720p
    auto fontPath = "src/resources/fonts/Comic Sans MS.ttf";
    auto textProgramId = gfxController.getProgramId(2).get();
    auto typeTime = 2.0f; // Seconds to draw lines
    auto wipeTime = 0.5f; // Seconds to perform text "wiping"
    auto cutoff = vec3(0.0f, 125.0f, 0.0f);
    // How many text boxes should be present? 3? Infinite?
    // Let's go with infinite for now... Then we can refine later...
    auto split = [](string val) {
        // Split val into a vector of "words"
        auto wordSize = 0;
        auto lastSpacePos = -1;
        queue<string> words;
        for (auto i = 0; i < static_cast<int>(val.length()); ++i) {
            if (val[i] != ' ') wordSize++;
            else {
                if (wordSize == 0) continue;
                assert(i - wordSize >= 0);
                words.push(val.substr(i - wordSize, wordSize));
                lastSpacePos = i;
                wordSize = 0;
            }
        }
        // Check if there's a word at the string's tail
        if (wordSize > 0) {
            words.push(val.substr(lastSpacePos + 1, val.length() - lastSpacePos));
        }
        return words;
    };

    auto makeline = [](queue<string> &words) {
        string line = "";
        // Pop some words off the queue to form the line
        for (auto i = 0; i < WORDS_PER_LINE; ++i) {
            if (words.empty()) break;
            line += words.front();
            words.pop();
            if (i < WORDS_PER_LINE - 1 || !words.empty()) {
                line += " ";
            }
        }
        return line;
    };
    // Determine how many text boxes we need - from my small UNSCIENTIFIC experiments,
    // it looks like we can fit around 8 words per line... so let's go with that.
    auto words = split(message);
    printf("Number of words: %lu\n", words.size());
    auto nLines = words.empty() ? 0 : (static_cast<int>(words.size()) / WORDS_PER_LINE) + 1; // Number of text lines = number of text boxes
    assert(nLines > 0);

    auto cb1 = [&words]() {
        printf("Running callback 1\n Words size is %lu\n", words.size());
    };

    auto grunty = currentGame->createSprite(sgrunty, vec3(-240.0f, 190.0f, 0.0f), 0.45f, gfxController.getProgramId(3).get(), "grunty");

    auto box = currentGame->createUi(textBoxImage, vec3(-220.0f, 150.0f, 0.0f), 0.7f, -50.0f, -50.0f, gfxController.getProgramId(4).get(), "textbox");

    KeyFrame *k0 = AnimationController::createKeyFrameCb(
        UPDATE_NONE,
        box->getPosition(),
        box->getStretch(),
        "",
        cb1,
        1.0f
    );
    KeyFrame *k1 = AnimationController::createKeyFrame(
        UPDATE_POS,
        vec3(30.0f, 150.0f, 0.0f),  // Position
        box->getStretch(),  // Stretch
        "",
        0.3f  // Time in SECONDS
    );
    KeyFrame *k2 = AnimationController::createKeyFrame(
        UPDATE_STRETCH,
        vec3(30.0f, 150.0f, 0.0f),
        vec3(1035, -50, 0.0f),
        "",
        0.5f
    );

    // Keyframes for Grunty
    KeyFrame *gk0 = AnimationController::createKeyFrame(
        UPDATE_NONE,
        grunty->getPosition(),
        grunty->getPosition(),  // Ignore for sprites
        "",
        1.0f
    );

    KeyFrame *gk1 = AnimationController::createKeyFrame(
        UPDATE_POS,
        vec3(10.0f, 190.0f, 0.0f),
        grunty->getPosition(),
        "",
        0.3f
    );

    renderer->addSceneObject(box);
    renderer->addSceneObject(grunty);
    
    animationController.addKeyframe(box, k0);
    animationController.addKeyframe(box, k1);
    animationController.addKeyframe(box, k2);

    animationController.addKeyframe(grunty, gk0);
    animationController.addKeyframe(grunty, gk1);

    
    queue<TextObject *>textLines;
    auto textBoxId = 0; // Make sure textbox names are UNIQUE
    auto textShiftTime = 1.8f;
    // Create the text objects
    for (int i = 0; i < nLines; ++i) {
        // Grab the line of text to use
        auto text = makeline(words);
        // The first text box will always be on the top
        auto textBox = currentGame->createText("",
            i == 0 ? topLine : bottomLine,
            textScale,
            fontPath,
            textProgramId,
            "MessageText" + std::to_string(textBoxId++));
        textBox->setCutoff(cutoff);
        textLines.push(textBox);
        renderer->addSceneObject(textBox);

        KeyFrame *kpause = nullptr;

        kpause = AnimationController::createKeyFrame(
            UPDATE_NONE,
            topLine,
            topLine,
            text,
            textShiftTime
        );

        // Now we animate the text boxes to draw text. Let's just do linear
        // drawing and then do something fancy later
        auto k0 = AnimationController::createKeyFrame(
            UPDATE_TEXT,
            topLine,
            topLine,
            text,
            typeTime
        );

        auto k1 = AnimationController::createKeyFrame(
            UPDATE_NONE,
            topLine,
            topLine,
            text,
            typeTime
        );

        auto k2 = AnimationController::createKeyFrame(
            UPDATE_POS,
            textBox->getPosition() + shift,
            topLine,
            text,
            wipeTime
        );
        textShiftTime += typeTime;
        if (kpause != nullptr) animationController.addKeyframe(textBox, kpause);
        animationController.addKeyframe(textBox, k0);
        animationController.addKeyframe(textBox, k1);
        animationController.addKeyframe(textBox, k2);

    };

    return 0;
    // Probs delete the generated text objects when we're done so we don't
    // need to worry about them breaking later
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

    textTest->setCutoff(vec3(0.0f, 125.0f, 0.0f));
    textTest1->setCutoff(vec3(0.0f, 125.0f, 0.0f));
    
    auto currentCamera = currentGame->createCamera(nullptr,
        vec3(5.140022f, 1.349999f, 2.309998f), 3.14159 / 5.0f, 16.0f / 9.0f, 4.0f, 90.0f);
    cout << "currentGameObject tag is NULLPTR" // playerRef->getObjectName()
        << '\n';

    showMessage("Hocus Pocus, Honus Loopus. Where have I seen this cryptic skinny dipping? LAKE, WATER, POOL, FENCE?", currentCamera, currentGame);

    fps_counter = fpsText;
    fps_counter->setMessage("FPS: 0");

    // Add objects to camera
    vector<SceneObject *> targets = {
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
    //thread messageRoutine(sendMessageRoutine, box, textTest, &currentGameInfo);
    mainLoop(&currentGameInfo);
    isDone = true;
    //rotThread.join();
    //messageRoutine.join();
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
