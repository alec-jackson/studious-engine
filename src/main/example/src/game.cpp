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
#include <random>
#include <atomic>
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

// Just to make this easier to understand...
#define ACCEPT_SFX_NUM 7
vector<string> soundList = {
    "src/resources/sfx/music/GruntyFurnace.mp3",
    "src/resources/sfx/Soundbox SFX.mp3",
    "src/resources/sfx/Grunty Witch SFX1.mp3",
    "src/resources/sfx/Grunty Witch SFX2.mp3",
    "src/resources/sfx/Grunty Witch SFX3.mp3",
    "src/resources/sfx/Select SFX.mp3",
    "src/resources/sfx/Song Snippet 1.mp3",
    "src/resources/sfx/Accept SFX.mp3",
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
std::atomic<int> optionsReady(0);
std::atomic<int> wordsSpoken(0);
std::atomic<int> uiElementsReset(0);
double deltaTime = 0.0f;
int WORDS_PER_LINE = 8; // Arbitrary
#ifdef GFX_EMBEDDED
OpenGlEsGfxController gfxController = OpenGlEsGfxController();
#else
OpenGlGfxController gfxController = OpenGlGfxController();
#endif
AnimationController animationController;

enum GameState {
    CHATTING, 
    ANSWERING,
    SHOWCASE,
    WAITING,
    CONFIRMING,
    CONFIRM_CHAT
};

enum QuestionType {
    MUSIC,
    TRIVIA,
    IMAGE
};

struct GameLogicInfo {
    GameInstance *currentGame;
    GameState currentState = WAITING;
    float currentDebounce = 0.0f;
    float debounceSeconds = 0.3f;
    int currentOption = 0;
    int prevOption = 0;
    float currentVolume = 50.0f;
    float maxSongVolume = 50.0f;
    float songTimePass = 2.0f;
    float currentSongTime = 0.0f;
    float volumeRampSeconds = 2.0f;
    float currentVolumeRampSeconds = 0.0f;
    int songChannel = -1;
};

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

int countWords(string val) {
    int wordCount = 0;
    auto wordSize = 0;
        for (auto i = 0; i < static_cast<int>(val.length()); ++i) {
            if (val[i] != ' ') wordSize++;
            else {
                if (wordSize == 0) continue;
                assert(i - wordSize >= 0);
                wordSize = 0;
                wordCount++;
            }
        }
        // Check if there's a word at the string's tail
        if (wordSize > 0) wordCount++;
        return wordCount;
    };

queue<SceneObject *> showMessage(string message, CameraObject *renderer, GameInstance *currentGame) {
    printf("showMessage: Entry\n");
    auto topLine = vec3(250.0f, 90.0f, 0.0f);
    auto bottomLine = vec3(250.0f, 45.0f, 0.0f);
    auto shift = vec3(0.0f, 50.0f, 0.0f); // Text transform when "wiping"
    auto textScale = 0.8f; // Good for 720p
    auto fontPath = "src/resources/fonts/Comic Sans MS.ttf";
    auto textProgramId = gfxController.getProgramId(2).get();
    auto typeTime = 3.0f; // Seconds to draw lines
    auto wipeTime = 0.5f; // Seconds to perform text "wiping"
    auto cutoff = vec3(0.0f, 250.0f, 0.0f);
    queue<SceneObject *> generatedObjects;

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

    auto cbTextNoise = [currentGame]() {
        currentGame->playSound(1, 0, 128);
    };

    auto grunty = currentGame->createSprite(sgrunty, vec3(-240.0f, 190.0f, 0.0f), 0.45f, gfxController.getProgramId(3).get(), "grunty");

    auto box = currentGame->createUi(textBoxImage, vec3(-220.0f, 150.0f, 0.0f), 0.7f, -50.0f, -50.0f, gfxController.getProgramId(4).get(), "textbox");

    generatedObjects.push(grunty);
    generatedObjects.push(box);

    KeyFrame *k0 = AnimationController::createKeyFrameCb(
        UPDATE_NONE,
        box->getPosition(),
        box->getStretch(),
        "",
        cbTextNoise,
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

    KeyFrame *gk2 = AnimationController::createKeyFrame(
        UPDATE_NONE,
        vec3(10.0f, 190.0f, 0.0f),
        grunty->getPosition(),
        "",
        0.5f
    );

    renderer->addSceneObject(box);
    renderer->addSceneObject(grunty);
    
    animationController.addKeyframe(box, k0);
    animationController.addKeyframe(box, k1);
    animationController.addKeyframe(box, k2);

    animationController.addKeyframe(grunty, gk0);
    animationController.addKeyframe(grunty, gk1);
    animationController.addKeyframe(grunty, gk2);

    
    queue<TextObject *>textLines;
    auto textBoxId = 0; // Make sure textbox names are UNIQUE
    auto textShiftTime = 1.8f;
    for (int i = 0; i < nLines; ++i) {
        // Determine type time by string length @todo
        // Grab the line of text to use
        auto text = makeline(words);
        // Split again for additional keyframes... you'll see
        auto lsplit = split(text);
        auto referenceLineLength = 50;
        // Tweak the type time for different lengths
        float tweakedTypeTime = typeTime * (static_cast<float>(text.length()) / static_cast<float>(referenceLineLength));

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
        generatedObjects.push(textBox);
        vec3 curPos = textBox->getPosition();

        auto cbVoice = [currentGame]() {
            static int lastchannel = -1;
            // Stop playing the last sound if set
            if (lastchannel != -1) {
                currentGame->stopSound(lastchannel);
            }
            std::random_device rd;
            std::mt19937 gen(rd());
            // Calculate random number between 2-4
            std::uniform_int_distribution<> dis(2, 4);

            int random_number = dis(gen);
            assert(random_number > 1 && random_number < 5);
            lastchannel = currentGame->playSound(random_number, 0, 50);
            printf("cbVoice: Playing sound%d\n", random_number);
            wordsSpoken++;
        };

        auto kf = AnimationController::createKeyFrame(
            UPDATE_NONE,
            topLine,
            topLine,
            text,
            textShiftTime
        );
        
        animationController.addKeyframe(textBox, kf);
        auto writeTime = i == nLines - 1 ? tweakedTypeTime : typeTime;
        string builtString = "";
        auto ogSize = lsplit.size();
        for (auto i = 0; i < ogSize; ++i) {
            builtString += lsplit.front() + " ";
            lsplit.pop();
            auto proportionaltime = writeTime / ogSize;
            // Add each word at a time...
            kf = AnimationController::createKeyFrameCb(
                UPDATE_TEXT,
                topLine,
                topLine,
                builtString,
                cbVoice,
                proportionaltime
            );
            animationController.addKeyframe(textBox, kf);
        }
        
        textShiftTime += writeTime;
        

        // If last, DONE
        if (i == nLines - 1) continue;

        // If First, PAUSE
        if (i == 0) {
            kf = AnimationController::createKeyFrame(
                UPDATE_NONE,
                topLine,
                topLine,
                text,
                typeTime
            );
            animationController.addKeyframe(textBox, kf);
        } else if (i != nLines - 1) {
            // If NOT last and NOT first - perform SHIFT
            curPos += shift;
            textShiftTime += wipeTime;
            kf = AnimationController::createKeyFrame(
                UPDATE_POS,
                curPos,
                topLine,
                text,
                wipeTime
            );
            animationController.addKeyframe(textBox, kf);
        }

        // If second to last, DONE
        if (i == nLines - 2) continue;
        
        // Otherwise...
        // If not first, PAUSE
        if (i != 0) {
            kf = AnimationController::createKeyFrame(
                UPDATE_NONE,
                topLine,
                topLine,
                text,
                typeTime
            );
            animationController.addKeyframe(textBox, kf);
        }

        // And wipe!
        curPos += shift;
        kf = AnimationController::createKeyFrame(
            UPDATE_POS,
            curPos,
            topLine,
            text,
            wipeTime
        );
        animationController.addKeyframe(textBox, kf);

    };

    return generatedObjects;
    // Probs delete the generated text objects when we're done so we don't
    // need to worry about them breaking later
}

void hideMessage(queue<SceneObject *> objects, GameInstance *currentGame) {
    printf("hideMessage: Entry\n");
    assert(objects.size() > 1);
    // Runs a routine to hide the on-screen message
    // Grunty should be the first object
    auto grunty = objects.front();
    objects.pop();
    assert(grunty->getObjectName().compare("grunty") == 0);
    auto box = objects.front();
    objects.pop();
    assert(box->getObjectName().compare("textbox") == 0);
    assert(box->type() == ObjectType::UI_OBJECT);
    auto cBox = static_cast<UiObject *>(box);

    auto cbTextNoise = [currentGame]() {
        currentGame->playSound(1, 0, 128);
    };

    auto cbElementResetG = [currentGame, grunty]() {
        currentGame->removeSceneObject(grunty->getObjectName());
        uiElementsReset++;
    };

    auto cbElementResetB = [currentGame, box]() {
        currentGame->removeSceneObject(box->getObjectName());
        uiElementsReset++;
    };
    
    KeyFrame *k0 = AnimationController::createKeyFrameCb(
        UPDATE_NONE,
        cBox->getPosition(),
        cBox->getStretch(),
        "",
        cbTextNoise,
        1.0f
    );
    KeyFrame *k1 = AnimationController::createKeyFrame(
        UPDATE_STRETCH,
        vec3(30.0f, 150.0f, 0.0f),  // Position
        vec3(-50.0f, -50.0f, 0.0f),  // Stretch
        "",
        0.5f  // Time in SECONDS
    );
    KeyFrame *k2 = AnimationController::createKeyFrameCb(
        UPDATE_POS,
        vec3(-220.0f, 150.0f, 0.0f),
        vec3(1035, -50, 0.0f),
        "",
        cbElementResetB,
        0.3f
    );

    // Keyframes for Grunty
    KeyFrame *gk0 = AnimationController::createKeyFrame(
        UPDATE_NONE,
        grunty->getPosition(),
        grunty->getPosition(),  // Ignore for sprites
        "",
        0.5f
    );

    KeyFrame *gk1 = AnimationController::createKeyFrameCb(
        UPDATE_POS,
        vec3(-240.0f, 190.0f, 0.0f),
        grunty->getPosition(),
        "",
        cbElementResetG,
        0.3f
    );

    //animationController.addKeyframe(box, k0);
    animationController.addKeyframe(box, k1);
    animationController.addKeyframe(box, k2);

    animationController.addKeyframe(grunty, gk0);
    animationController.addKeyframe(grunty, gk1);

    // Will need to perform cleanup in callbacks...

    while (!objects.empty()) {
        // Pop the text objects off the queue and delete them
        auto object = objects.front();
        objects.pop();
        currentGame->removeSceneObject(object->getObjectName());
    }
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
    
    auto currentCamera = currentGame->createCamera(nullptr,
        vec3(5.140022f, 1.349999f, 2.309998f), 3.14159 / 5.0f, 16.0f / 9.0f, 4.0f, 90.0f);
    cout << "currentGameObject tag is NULLPTR" // playerRef->getObjectName()
        << '\n';
    
    auto backdrop = currentGame->createSprite(
        "src/resources/images/Furnace Fun Image.png",
        vec3(-300.0f, 900.0f, 0.0f),
        1.0f,
        gfxController.getProgramId(3).get(),
        "Backdrop"
    );

    

    fps_counter = fpsText;
    fps_counter->setMessage("FPS: 0");

    // Add objects to camera
    vector<SceneObject *> targets = {
        backdrop,
        fpsText
    };

    // Add all objects to active camera
    for (auto it = targets.begin(); it != targets.end(); ++it) {
        cout << "Adding to camera: " << (*it)->getObjectName() << endl;
        currentCamera->addSceneObject(*it);
    }

    currentGameInfo.isDone = &isDone;
    currentGameInfo.gameCamera = currentCamera;
    currentGameInfo.currentGame = currentGame;
    
    //currentGame->changeWindowMode(SDL_WINDOW_FULLSCREEN);
    /*
     End Scene Loading
     */
    // Additional threads should be added, pipes will most likely be required
    // Might also be a good idea to keep the parent thread local to watch for
    // unexpected failures and messages from children
    //thread rotThread(rotateShape, &currentGameInfo, playerRef);
    //thread messageRoutine(showMessage, "Hee-hee-hee! Hear that tune, it's quite a mystery, whose theme song could it be?", currentCamera, currentGame);
    mainLoop(&currentGameInfo);
    isDone = true;
    //rotThread.join();
    //messageRoutine.join();
    cout << "Running cleanup\n";
    currentGame->cleanup();
    return 0;
}

void hideOptions(vector<SceneObject *> objects, GameLogicInfo *game) {
    printf("hideOptions: Entry\n");
    for (auto object : objects) {
        auto objectCleanupCb = [object, game]() {
            game->currentGame->removeSceneObject(object->getObjectName());
        };
        // Check if not text box
        if (object->type() == ObjectType::UI_OBJECT) {
            // Key frame and then delete
            // Key frames for PAF button are special
            if (object->getObjectName().compare("option4") == 0) {
                auto endPafButton = vec3(1030.0f, 400.0f, 0.0f);
                auto midPafButton = vec3(1130.0f, 300.0f, 0.0f);
                auto startPafButton = vec3(1280.0f, 300.0f, 0.0f);
                auto pafStretchStart = vec3(-50.0f, -50.0f, 0.0f);
                auto pafStretchEnd = vec3(50.0f, 50.0f, 0.0f);
                auto kf = AnimationController::createKeyFrame(
                    UPDATE_STRETCH | UPDATE_POS,
                    midPafButton,
                    pafStretchStart, 
                    "",
                    0.3f
                );
                animationController.addKeyframe(object, kf);

                kf = AnimationController::createKeyFrameCb(
                    UPDATE_POS,
                    startPafButton,
                    startPafButton,
                    "",
                    objectCleanupCb,
                    0.2f
                );
                animationController.addKeyframe(object, kf);
            } else {
                auto startStretch = vec3(-50.0f, -50.0f, 0.0f);
                auto endShift = vec3(440.0f, 0.0f, 0.0f);
                auto kf = AnimationController::createKeyFrame(
                    UPDATE_STRETCH,
                    startStretch,
                    startStretch, 
                    "",
                    0.3f
                );
                animationController.addKeyframe(object, kf);
                kf = AnimationController::createKeyFrameCb(
                    UPDATE_POS,
                    object->getPosition() - endShift,
                    startStretch,
                    "",
                    objectCleanupCb,
                    0.2f
                );
                animationController.addKeyframe(object, kf);   
            }
        } else {
            // Just delete the text objects
            game->currentGame->removeSceneObject(object->getObjectName());
        }
    }
}

vector<SceneObject *> drawOptions(vector<string> options, gameInfo *gamein) {
    vector<SceneObject *>uiElements;
    auto currentGame = gamein->currentGame;
    auto shift = vec3(0.0f, 130.0f, 0.0f);
    auto currentShift = vec3(0.0f, 0.0f, 0.0f);
    auto startPos = vec3(-140.0f, 300.0f, 0.0f);
    auto textPosStart = vec3(380.0f, 90.0f, 0.0f);
    auto endShift = vec3(440.0f, 0.0f, 0.0f);
    auto startStretch = vec3(-50.0f, -50.0f, 0.0f);
    auto endStretch = vec3(500.0f, -50.0f, 0.0f);
    auto textScale = 0.8f; // Good for 720p
    auto fontPath = "src/resources/fonts/Comic Sans MS.ttf";
    auto textProgramId = gfxController.getProgramId(2).get();

    // Increment to "signal" completion
    auto incrementOptCb = []() {
        optionsReady++;
    };

    auto cbTextNoise = [currentGame]() {
        currentGame->playSound(1, 0, 128);
    };

    // Present up to four options to the user - use cbs to determine when done
    auto currentOptionIndex = 0;
    for (auto option : options) {
        // Draw each option in a text box
        auto box = currentGame->createUi(textBoxImage, startPos + currentShift, 0.7f, startStretch.x, startStretch.y, gfxController.getProgramId(4).get(), "option" + std::to_string(currentOptionIndex));
        currentShift += shift;
        gamein->gameCamera->addSceneObject(box);
        uiElements.push_back(box);

        // Create animations for each text box
        auto k0 = AnimationController::createKeyFrameCb(
            UPDATE_NONE,
            startPos,
            startPos,
            "",
            cbTextNoise,
            1.0f
        );

        auto k1 = AnimationController::createKeyFrame(
            UPDATE_POS,
            box->getPosition() + endShift,
            startPos,
            "",
            0.2f
        );

        auto k2 = AnimationController::createKeyFrame(
            UPDATE_STRETCH,
            startPos,
            endStretch, 
            "",
            0.3f
        );

        animationController.addKeyframe(box, k0);
        animationController.addKeyframe(box, k1);
        animationController.addKeyframe(box, k2);

        // Draw option text on the button
        auto textBox = currentGame->createText("",
            textPosStart + currentShift,
            textScale,
            fontPath,
            textProgramId,
            "OptionText" + std::to_string(currentOptionIndex++));
        textBox->setColor(vec3(0.3f, 0.3f, 0.3f));

        gamein->gameCamera->addSceneObject(textBox);
        uiElements.push_back(textBox);

        // Start writing text after 2.0f seconds
        auto t0 = AnimationController::createKeyFrame(
            UPDATE_NONE,
            textPosStart,
            textPosStart,
            "",
            1.5f
        );

        auto t1 = AnimationController::createKeyFrameCb(
            UPDATE_TEXT,
            textPosStart,
            textPosStart,
            option,
            incrementOptCb,
            0.3f
        );

        animationController.addKeyframe(textBox, t0);
        animationController.addKeyframe(textBox, t1);
    }

    auto endPafButton = vec3(1030.0f, 400.0f, 0.0f);
    auto midPafButton = vec3(1130.0f, 300.0f, 0.0f);
    auto startPafButton = vec3(1280.0f, 300.0f, 0.0f);
    auto pafStretchStart = vec3(-50.0f, -50.0f, 0.0f);
    auto pafStretchEnd = vec3(50.0f, 50.0f, 0.0f);
    auto pafBox = currentGame->createUi(textBoxImage, startPafButton, 0.7f, pafStretchStart.x, pafStretchStart.y, gfxController.getProgramId(4).get(), "option" + std::to_string(currentOptionIndex));
    gamein->gameCamera->addSceneObject(pafBox);
    uiElements.push_back(pafBox);

    // Create animations for each text box
    auto k0 = AnimationController::createKeyFrame(
        UPDATE_NONE,
        startPos,
        startPos,
        "",
        1.0f
    );

    auto k1 = AnimationController::createKeyFrame(
        UPDATE_POS,
        midPafButton,
        startPos,
        "",
        0.2f
    );

    auto k2 = AnimationController::createKeyFrame(
        UPDATE_STRETCH | UPDATE_POS,
        endPafButton,
        pafStretchEnd, 
        "",
        0.3f
    );

    animationController.addKeyframe(pafBox, k0);
    animationController.addKeyframe(pafBox, k1);
    animationController.addKeyframe(pafBox, k2);
    auto textPosStartPaf = vec3(1060.0f, 300.0f, 0.0f);

    // Draw the text on the box
    // Draw option text on the button
    auto textBox = currentGame->createText("",
        textPosStartPaf,
        textScale,
        fontPath,
        textProgramId,
        "OptionText" + std::to_string(currentOptionIndex++));
    textBox->setColor(vec3(0.3f, 0.3f, 0.3f));
    uiElements.push_back(textBox);

    gamein->gameCamera->addSceneObject(textBox);
    uiElements.push_back(textBox);

    // Start writing text after 2.0f seconds
    auto t0 = AnimationController::createKeyFrame(
        UPDATE_NONE,
        textPosStart,
        textPosStart,
        "",
        1.5f
    );

    auto t1 = AnimationController::createKeyFrameCb(
        UPDATE_TEXT,
        textPosStart,
        textPosStart,
        "  PHONE\nA FRIEND",
        incrementOptCb,
        0.3f
    );

    animationController.addKeyframe(textBox, t0);
    animationController.addKeyframe(textBox, t1);
    return uiElements;
}

bool debounceCheck(GameLogicInfo *game) {
    return game->currentDebounce > game->debounceSeconds;
}

int selectionHandler(GameLogicInfo *game) {
    int result = -1;
    if (game->currentGame->getKeystate()[SDL_SCANCODE_W] && debounceCheck(game)) {
        // "up" action
        // Change the selected option up to 3
        if (game->currentOption < 3) {
            game->currentOption++;
            game->currentGame->playSound(5, 0, 50);
        }
        game->currentDebounce = 0.0f;
    } else if (game->currentGame->getKeystate()[SDL_SCANCODE_S] && debounceCheck(game)) {
        // "up" action
        // Change the selected option up to 3
        if (game->currentOption > 0) {
            game->currentOption--;
            game->currentGame->playSound(5, 0, 50);
        }
        game->currentDebounce = 0.0f;
    } else if (game->currentGame->getKeystate()[SDL_SCANCODE_D] && debounceCheck(game)) {
        // "right" action
        // Switch to option 4
        if (game->currentOption != 4) {
            game->prevOption = game->currentOption;
            game->currentOption = 4;
            game->currentGame->playSound(5, 0, 50);
        }
        game->currentDebounce = 0.0f;
    } else if (game->currentGame->getKeystate()[SDL_SCANCODE_A] && debounceCheck(game)) {
        // "left" action
        // Change the selected option back from "PAF"
        if (game->currentOption == 4) {
            game->currentOption = game->prevOption;
            game->currentGame->playSound(5, 0, 50);
        }
        game->currentDebounce = 0.0f;
    } else if (game->currentGame->getKeystate()[SDL_SCANCODE_RETURN] && debounceCheck(game)) {
        // "SELECT" action
        result = game->currentOption;
        game->currentGame->playSound(ACCEPT_SFX_NUM, 0, 100);
    }
    return result;
}

/*
 (int) mainLoop starts rendering objects in the current GameInstance to the
 main SDL window. The methods called from the currentGame object render parts
 of the scene.

 (int) mainLoop returns 0 when closed successfully. When an error occurs and the
 mainLoop closes prematurely, an error code is returned.
*/
int mainLoop(gameInfo* gamein) {
    auto gameState = WAITING;
    Uint64 begin, end;
    int running = 1;
    double currentTime = 0.0, sampleTime = 1.0;
    GameInstance *currentGame = gamein->currentGame;
    int error = 0;
    vector<double> times;
    bool hasHidden = false;
    double timePassed = 0.0;
    // Going to just make the entire game in the main loop
    vector<string> options = {
        "Answer 1",
        "Answer 2",
        "Answer 3",
        "Answer 4"
    };

    /**
     * Types of QUESTIONS we can be asked:
     * * Name that tune.
     * * General trivia question.
     * * Name that image.
     */
    auto songStarted = false;
    GameLogicInfo game;
    game.currentGame = currentGame;
    string firstMessage = "Hee-hee-hee! Hear that tune, it's quite a mystery, whose theme song could it be?";
    auto firstMessageWords = countWords(firstMessage);
    int selectionHandlerResult;
    vector<SceneObject *> uiObjects;
    queue<SceneObject *>chatObjectCache;
    string answer = "";
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
        //gameLogicLoop(gamein);
        deltaTime = static_cast<double>(end - begin) / (SDL_GetPerformanceFrequency());
        currentGame->setDeltaTime(deltaTime);
        timePassed += deltaTime;
        game.currentDebounce += deltaTime;

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

        /**
         * Game logic loop
         */

        switch (gameState) {
            case WAITING:
                // If nothing is happening (usually waiting for wheel input)
                chatObjectCache = showMessage(firstMessage, gamein->gameCamera, gamein->currentGame);
                gameState = CHATTING;
                break;
            case CHATTING:
                // Wait for the chatting to end for the showcase step
                if (wordsSpoken == firstMessageWords) {
                    gameState = SHOWCASE;
                    wordsSpoken = 0;
                }
                break;
            case SHOWCASE:
                if (game.currentVolume > 0.0f) {
                    game.currentVolumeRampSeconds += deltaTime;
                    auto volFrac = game.currentVolumeRampSeconds / game.volumeRampSeconds;
                    game.currentVolume = game.maxSongVolume - game.maxSongVolume * volFrac;
                    currentGame->changeVolume(0, game.currentVolume);
                } else if (!songStarted) {
                    // Start playing the jesus song
                    game.songChannel = currentGame->playSound(6, 0, 100);
                    songStarted = true;
                } else {
                    if (game.currentSongTime < game.songTimePass) {
                        game.currentSongTime += deltaTime;
                    } else {
                        songStarted = false;
                        currentGame->stopSound(game.songChannel);
                        
                        uiObjects = drawOptions(options, gamein);
                        currentGame->changeVolume(0, game.maxSongVolume);
                        game.currentVolume = game.maxSongVolume;
                        gameState = ANSWERING;
                    }
                }
                break;
            case ANSWERING:
                // "dim" unselected options 
                if (optionsReady == 5) {
                    selectionHandlerResult = selectionHandler(&game);
                    auto dimColor = vec3(0.3f, 0.3f, 0.3f);
                    auto brightColor = vec3(1.0f);
                    string key = "OptionText";
                    vector<TextObject *> texts;
                    // Dim all optiontext objects
                    for (int i = 0; i < 5; ++i) {
                        auto obj = currentGame->getSceneObject(key + std::to_string(i));
                        assert(obj != nullptr);
                        texts.push_back(static_cast<TextObject *>(obj));
                    }

                    for (auto sel : texts) {
                        sel->setColor(dimColor);
                    }

                    // Brighten the selected one
                    texts.at(game.currentOption)->setColor(brightColor);
                    if (selectionHandlerResult != -1) {
                        // Perform selection operation
                        gameState = CONFIRMING;
                        optionsReady = 0;
                        hideOptions(uiObjects, &game);
                        hideMessage(chatObjectCache, game.currentGame);
                        if (selectionHandlerResult == 4) {
                            answer = "PHONE A FRIEND";
                        } else {
                            answer = options[selectionHandlerResult];
                        }
                        
                        printf("mainLoop: User answered %s\n", answer.c_str());
                    }
                }
                break;
            case CONFIRMING:
                // Wait for grunty dialoge to dismiss
                if (uiElementsReset != 2) break;
                if (answer.compare("Answer 1") != 0) {
                    chatObjectCache = showMessage("Guffaw, fool! That answer's wrong, oh boo-hoo! Sucks to be you, my dear—better luck next time, too!",
                        gamein->gameCamera, gamein->currentGame);
                } else {
                    chatObjectCache = showMessage("Well, well, well, you got it right, but don't think I'm impressed! That's the correct answer, much to my distress! Humph!", gamein->gameCamera, gamein->currentGame);
                }
                gameState = CONFIRM_CHAT;
                break;
            case CONFIRM_CHAT:
                
                break;
            default:
                printf("mainLoop: ERROR CASE\n");
                assert(false);
                break;
        }
    }
    return 0;
}

