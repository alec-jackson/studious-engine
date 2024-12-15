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
#include <queue>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
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

vector<string> hurtSounds = {
    "src/resources/sfx/Hurt 1.mp3",
    "src/resources/sfx/Hurt 2.mp3",
    "src/resources/sfx/Hurt 3.mp3",
    "src/resources/sfx/Hurt 4.mp3",
    "src/resources/sfx/Hurt 5.mp3",
    "src/resources/sfx/Hurt 6.mp3",
    "src/resources/sfx/Hurt 7.mp3"
};  // A list of sounds that can play when you take damage

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

const char *textBoxImage =
    "src/resources/images/Banjo Textbox.png";

const char *sgrunty =
    "src/resources/images/Scuffed Grunty.png";

const char *honeycombFull =
    "src/resources/images/Honeycomb.png";

const char *team1 =
    "src/resources/images/Team 1.png";

const float BG_VOLUME = 50.0f;
const float BG_RAMP_SECONDS = 2.0f;
const int MAX_HEALTH = 5;

TeamStats teamStats[3] = {
    {MAX_HEALTH - 1, 1},  // 0 = Team 1
    {MAX_HEALTH, 1},  // 1 = Team 2
    {MAX_HEALTH, 1}   // 2 = Team 3
};


TextObject *fps_counter;
TextObject *collDebugText;
TextObject *pressUText;
// This is kind of bad, but I am going to store showcase images as a global
SpriteObject *showcaseImage = nullptr;
std::atomic<int> optionsReady(0);
std::atomic<int> wordsSpoken(0);
std::atomic<int> uiElementsReset(0);
std::atomic<int> healthShown(0);
std::atomic<int> healthHidden(0);
int wordCount;
double deltaTime = 0.0f;
int WORDS_PER_LINE = 8;  // Arbitrary
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

void playRandomHurtSound(GameInstance *currentGame) {
    printf("PLAYING RANDOM HURT SOUND!!!\n");
    // Grab a random hurt sound from the hurt sfx
    std::random_device rd;
    std::mt19937 gen(rd());
    // Calculate random number between 2-4
    std::uniform_int_distribution<> dis(0, hurtSounds.size() - 1);

    int random_number = dis(gen);
    assert(random_number >= 0 && random_number < hurtSounds.size());
    // Play the hurt sound determined with the random number
    currentGame->playSound(hurtSounds.at(random_number).c_str(), 128);
}

vector<SceneObject *> showTeamHealth(int teamNumber, CameraObject *renderer, GameInstance *currentGame) {
    printf("showTeamHealth: Entry - teamNumber %d\n", teamNumber);
    float hcDisplacement = 60.0f;
    float startingX = -90.0f;
    float leftEndX = 500.0f;
    vector<SceneObject *> objectCache;

    float endStretch = 320.0f;
    float startStretch = -50.0f;
    float endBox = 355.0f;
    vec3 boxPos = vec3(-140.0f, 425.0f, 0.0f);
    vec3 boxStretch = vec3(startStretch, startStretch, 0.0f);

    // Grab the current health for this team
    assert(teamNumber > 0);
    assert(teamNumber < (sizeof(teamStats) / sizeof(TeamStats)));

    auto currentHealth = teamStats[teamNumber - 1].teamHealth;

    // Team 1 Image
    vec3 t1EndPos = vec3(380.0f, 410.0f, 0.0f);
    vec3 t1StartPos = vec3(-150.0f, 410.0f, 0.0f);


    // Create textbox for healthbar backdrop
    auto box = currentGame->createUi(textBoxImage,
        boxPos,
        0.7f,
        startStretch,
        -50.0f,
        gfxController.getProgramId(4).get(),
    "healthbg");

    renderer->addSceneObject(box);

    auto t1 = currentGame->createSprite(team1,
        t1StartPos,
        0.40f,
        gfxController.getProgramId(3).get(),
        SpriteAnchor::BOTTOM_LEFT,
        "team1");

    // Programatically create health bar icons
    for (int i = 0; i < MAX_HEALTH; ++i) {
        auto honeyComb = currentGame->createSprite(
            honeycombFull,
            vec3(startingX, 400.0f, 0.0f),
            0.45f,
            gfxController.getProgramId(3).get(),
            SpriteAnchor::BOTTOM_LEFT,
            "hc" + std::to_string(i));

        // Based in the team's current health, change the tint of each honeycomb
        if (i > currentHealth - 1) {
            honeyComb->setTint(vec3(-0.5));
        }
        objectCache.push_back(honeyComb);
    }

    // Add honeycomb objects to renderer in reverse order (for correct rendering)
    for (int i = objectCache.size() - 1; i >= 0; --i) {
        renderer->addSceneObject(objectCache.at(i));
    }

    // Add team object last (top render)
    renderer->addSceneObject(t1);

    // Create animations for each health icon
    float leftReachTime = 0.7f;
    float deltaSlideTime = 0.1f;
    float maxDeltaSlide = 0.0f;
    auto i = 0;
    // Increment atomic count when health icons done animating
    auto cb = []() {
        healthShown++;
    };

    // Create the initial slide to the right keyframe
    auto kf0 = AnimationController::createKeyFrameCb(
        UPDATE_POS,
        t1EndPos,
        vec3(0),
        "",
        cb,
        leftReachTime);

    // Create a kf1 based on the deltaSlideTime and displacement
    // Add the keyframes to the animation controller
    animationController.addKeyFrame(t1, kf0);

    // Iterate through each honeycomb object and create animations for them
    for (auto hc : objectCache) {
        auto destination = hc->getPosition();
        destination.x = leftEndX;
        // Create the initial slide to the right keyframe
        auto kf0 = AnimationController::createKeyFrame(
            UPDATE_POS,
            destination,
            vec3(0),
            "",
            leftReachTime);

        auto displacement = (i * hcDisplacement);
        auto postSlideTime = ((i + 1) * deltaSlideTime);
        // Update the destination with the displacement
        destination.x += displacement;

        auto kf1 = AnimationController::createKeyFrameCb(
            UPDATE_POS,
            destination,
            vec3(0),
            "",
            cb,
            postSlideTime);
        // Create a kf1 based on the deltaSlideTime and displacement
        // Add the keyframes to the animation controller
        animationController.addKeyFrame(hc, kf0);
        animationController.addKeyFrame(hc, kf1);
        ++i;
        maxDeltaSlide = postSlideTime;
    }

    boxPos.x = endBox;
    // Create keyframes for textbox with timing from health icons
    auto kfbox = AnimationController::createKeyFrame(
        UPDATE_POS,
        boxPos,
        vec3(0),
        "",
        leftReachTime);

    boxStretch.x = endStretch;
    auto kf1box = AnimationController::createKeyFrameCb(
        UPDATE_STRETCH,
        vec3(0),
        boxStretch,
        "",
        cb,
        maxDeltaSlide);

    animationController.addKeyFrame(box, kfbox);
    animationController.addKeyFrame(box, kf1box);

    // Add the background box and team image after honeycombs in objectCache
    objectCache.push_back(box);
    objectCache.push_back(t1);

    return objectCache;
}

void updateHealthIndicator(vector<SceneObject *> objectCache, int health) {
    // Color in honeycombs based on health
    assert(health <= MAX_HEALTH);
    // Iterate through honeycomb objects and reverse their keyframes
    // Iterate through each honeycomb object and create animations for them
    for (int i = 0; i < MAX_HEALTH; ++i) {
        auto hc = objectCache.at(i);
        // Cast to sprite object
        auto chc = static_cast<SpriteObject *>(hc);
        // Set the tint
        if (i + 1 > health) {
            chc->setTint(vec3(-0.5f));
        } else {
            chc->setTint(vec3(0.0f));
        }
    }
}

void hideTeamHealth(vector<SceneObject *> objectCache) {
    // This is kind of backwards, but I am copying constants from showTeamHealth for now
    float hcDisplacement = 60.0f;
    float startingX = -90.0f;
    float leftEndX = 440.0f;

    float endStretch = 320.0f;
    float startStretch = -50.0f;
    float endBox = 355.0f;
    vec3 boxPos = vec3(-140.0f, 425.0f, 0.0f);
    vec3 boxStretch = vec3(startStretch, startStretch, 0.0f);

    float leftReachTime = 0.7f;
    float deltaSlideTime = 0.1f;
    float maxDeltaSlide = 0.0f;

    // Team 1 Image
    vec3 t1EndPos = vec3(380.0f, 410.0f, 0.0f);
    vec3 t1StartPos = vec3(-150.0f, 410.0f, 0.0f);
    // Callback to help determine when animation is complete
    auto cb = []() {
        healthHidden++;
    };
    // The object cache should have the following objects in order
    // Honeycombs (0 - MAX_HEALTH)
    // background box
    // team image
    // So there HAS TO BE MAX_HEALTH + 2 objects in the cache total
    assert(objectCache.size() == MAX_HEALTH + 2);

    // Iterate through honeycomb objects and reverse their keyframes
    // Iterate through each honeycomb object and create animations for them
    for (int i = 0; i < MAX_HEALTH; ++i) {
        auto hc = objectCache.at(i);
        auto destination = hc->getPosition();
        // We will need to subtract displacement from destination
        auto displacement = (i * hcDisplacement);
        destination.x -= displacement;  // Should be zero for first hc

        // The initial sleep time is based on the postSlideTime for other hcs
        auto postSlideTime = (i * deltaSlideTime);
        auto sleepSlideTime = ((MAX_HEALTH - 1) * deltaSlideTime) - postSlideTime;
        // Create the initial slide to the left keyframe
        auto kf0 = AnimationController::createKeyFrame(
            UPDATE_POS,
            destination,
            vec3(0),
            "",
            postSlideTime);

        // The next keyframe is the sleep time
        auto kf1 = AnimationController::createKeyFrame(
            UPDATE_NONE,
            vec3(0),
            vec3(0),
            "",
            sleepSlideTime);

        // Then ultimately the slide to the left frame (original start)
        destination.x = startingX;
        auto kf2 = AnimationController::createKeyFrameCb(
            UPDATE_POS,
            destination,
            vec3(0),
            "",
            cb,
            leftReachTime);
        // Add the keyframes to the animation controller
        animationController.addKeyFrame(hc, kf0);
        animationController.addKeyFrame(hc, kf1);
        animationController.addKeyFrame(hc, kf2);
    }
    maxDeltaSlide = ((MAX_HEALTH - 1) * deltaSlideTime);

    // Grab the box from the objectCache
    auto box = objectCache.at(MAX_HEALTH);

    // Reverse the background health image animation
    // Shrink the text box and hide it
    auto kfbox = AnimationController::createKeyFrame(
        UPDATE_STRETCH,
        vec3(0),
        boxStretch,
        "",
        maxDeltaSlide);

    boxStretch.x = endStretch;
    auto kf1box = AnimationController::createKeyFrameCb(
        UPDATE_POS,
        boxPos,
        vec3(0),
        "",
        cb,
        leftReachTime);

    animationController.addKeyFrame(box, kfbox);
    animationController.addKeyFrame(box, kf1box);

    // Grab the team image from the objectCache
    auto teamImage = objectCache.at(MAX_HEALTH + 1);

    // Hide the team image with the same timing as everything else
    auto teamKf = AnimationController::createKeyFrame(
        UPDATE_NONE,
        vec3(0),
        vec3(0),
        "",
        maxDeltaSlide);

    auto teamKf1 = AnimationController::createKeyFrameCb(
        UPDATE_POS,
        t1StartPos,
        vec3(0),
        "",
        cb,
        leftReachTime);

    // Add the keyframes for the team image
    animationController.addKeyFrame(teamImage, teamKf);
    animationController.addKeyFrame(teamImage, teamKf1);
}

queue<SceneObject *> showMessage(string message, CameraObject *renderer, GameInstance *currentGame) {
    printf("showMessage: Entry\n");
    auto topLine = vec3(250.0f, 90.0f, 0.0f);
    auto bottomLine = vec3(250.0f, 45.0f, 0.0f);
    auto shift = vec3(0.0f, 50.0f, 0.0f);  // Text transform when "wiping"
    auto textScale = 0.8f;  // Good for 720p
    auto fontPath = "src/resources/fonts/Comic Sans MS.ttf";
    auto textProgramId = gfxController.getProgramId(2).get();
    auto typeTime = 3.0f;  // Seconds to draw lines
    auto wipeTime = 0.5f;  // Seconds to perform text "wiping"
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
            if (val[i] != ' ') {
                wordSize++;
            } else {
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
    wordCount = words.size();
    printf("Number of words: %lu\n", words.size());
    // Number of text lines = number of text boxes
    auto nLines = words.empty() ? 0 : (static_cast<int>(words.size()) / WORDS_PER_LINE);
    if (words.size() > 0 && words.size() % WORDS_PER_LINE != 0) {
        nLines += 1; // Add an extra line to avoid word size / WORDS_PER_LINE = 0 (trunc)
    }
    printf("Number of lines: %d\n", nLines);
    assert(nLines > 0);

    auto cbTextNoise = [currentGame]() {
        currentGame->playSound(1, 0, 128);
    };

    auto grunty = currentGame->createSprite(
        sgrunty,
        vec3(-240.0f, 190.0f, 0.0f),
        0.45f,
        gfxController.getProgramId(3).get(),
        SpriteAnchor::BOTTOM_LEFT,
    "grunty");

    auto box = currentGame->createUi(textBoxImage,
        vec3(-220.0f, 150.0f, 0.0f),
        0.7f,
        -50.0f,
        -50.0f,
        gfxController.getProgramId(4).get(),
    "textbox");

    generatedObjects.push(grunty);
    generatedObjects.push(box);

    KeyFrame *k0 = AnimationController::createKeyFrameCb(
        UPDATE_NONE,
        box->getPosition(),  ///@todo change these to vec3(0)
        box->getStretch(),
        "",
        cbTextNoise,
        1.0f);
    KeyFrame *k1 = AnimationController::createKeyFrame(
        UPDATE_POS,
        vec3(30.0f, 150.0f, 0.0f),  // Position
        box->getStretch(),  // Stretch
        "",
        0.3f);  // Time in SECONDS
    KeyFrame *k2 = AnimationController::createKeyFrame(
        UPDATE_STRETCH,
        vec3(30.0f, 150.0f, 0.0f),
        vec3(1035, -50, 0.0f),
        "",
        0.5f);

    // Keyframes for Grunty
    KeyFrame *gk0 = AnimationController::createKeyFrame(
        UPDATE_NONE,
        grunty->getPosition(),
        grunty->getPosition(),  // Ignore for sprites
        "",
        1.0f);

    KeyFrame *gk1 = AnimationController::createKeyFrame(
        UPDATE_POS,
        vec3(10.0f, 190.0f, 0.0f),
        grunty->getPosition(),
        "",
        0.3f);

    KeyFrame *gk2 = AnimationController::createKeyFrame(
        UPDATE_NONE,
        vec3(10.0f, 190.0f, 0.0f),
        grunty->getPosition(),
        "",
        0.5f);

    renderer->addSceneObject(box);
    renderer->addSceneObject(grunty);

    animationController.addKeyFrame(box, k0);
    animationController.addKeyFrame(box, k1);
    animationController.addKeyFrame(box, k2);

    animationController.addKeyFrame(grunty, gk0);
    animationController.addKeyFrame(grunty, gk1);
    animationController.addKeyFrame(grunty, gk2);

    queue<TextObject *>textLines;
    auto textBoxId = 0;  // Make sure textbox names are UNIQUE
    auto textShiftTime = 1.8f;
    for (int i = 0; i < nLines; ++i) {
        // Determine type time by string length @todo
        // Grab the line of text to use
        auto text = makeline(words);
        // Split again for additional keyframes... you'll see
        auto lsplit = split(text);
        auto referenceLineLength = 50;
        // Tweak the type time for different lengths
        float tweakedTypeTime =
            typeTime * (static_cast<float>(text.length()) / static_cast<float>(referenceLineLength));

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
            textShiftTime);

        animationController.addKeyFrame(textBox, kf);
        auto writeTime = i == nLines - 1 ? tweakedTypeTime : typeTime;
        string builtString = "";
        auto ogSize = lsplit.size();
        for (unsigned int i = 0; i < ogSize; ++i) {
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
                proportionaltime);
            animationController.addKeyFrame(textBox, kf);
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
                typeTime);
            animationController.addKeyFrame(textBox, kf);
        } else if (i != nLines - 1) {
            // If NOT last and NOT first - perform SHIFT
            curPos += shift;
            textShiftTime += wipeTime;
            kf = AnimationController::createKeyFrame(
                UPDATE_POS,
                curPos,
                topLine,
                text,
                wipeTime);
            animationController.addKeyFrame(textBox, kf);
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
                typeTime);
            animationController.addKeyFrame(textBox, kf);
        }

        // And wipe!
        curPos += shift;
        kf = AnimationController::createKeyFrame(
            UPDATE_POS,
            curPos,
            topLine,
            text,
            wipeTime);
        animationController.addKeyFrame(textBox, kf);
    }

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

    auto cbElementResetG = [currentGame, grunty]() {
        currentGame->removeSceneObject(grunty->getObjectName());
        uiElementsReset++;
    };

    auto cbElementResetB = [currentGame, box]() {
        currentGame->removeSceneObject(box->getObjectName());
        uiElementsReset++;
    };

    KeyFrame *k1 = AnimationController::createKeyFrame(
        UPDATE_STRETCH,
        vec3(30.0f, 150.0f, 0.0f),  // Position
        vec3(-50.0f, -50.0f, 0.0f),  // Stretch
        "",
        0.5f);  // Time in SECONDS

    KeyFrame *k2 = AnimationController::createKeyFrameCb(
        UPDATE_POS,
        vec3(-220.0f, 150.0f, 0.0f),
        vec3(1035, -50, 0.0f),
        "",
        cbElementResetB,
        0.3f);

    // Keyframes for Grunty
    KeyFrame *gk0 = AnimationController::createKeyFrame(
        UPDATE_NONE,
        grunty->getPosition(),
        grunty->getPosition(),  // Ignore for sprites
        "",
        0.5f);

    KeyFrame *gk1 = AnimationController::createKeyFrameCb(
        UPDATE_POS,
        vec3(-240.0f, 190.0f, 0.0f),
        grunty->getPosition(),
        "",
        cbElementResetG,
        0.3f);

    animationController.addKeyFrame(box, k1);
    animationController.addKeyFrame(box, k2);

    animationController.addKeyFrame(grunty, gk0);
    animationController.addKeyFrame(grunty, gk1);

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

    auto backdrop = currentGame->createSprite(
        "src/resources/images/Furnace Fun Image.png",
        vec3(-300.0f, 900.0f, 0.0f),
        1.0f,
        gfxController.getProgramId(3).get(),
        SpriteAnchor::BOTTOM_LEFT,
        "Backdrop");
    // Set the backdrop to the lowest priority for rendering
    backdrop->setRenderPriority(RenderPriority::LOW);

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
                auto midPafButton = vec3(1130.0f, 300.0f, 0.0f);
                auto startPafButton = vec3(1280.0f, 300.0f, 0.0f);
                auto pafStretchStart = vec3(-50.0f, -50.0f, 0.0f);
                auto kf = AnimationController::createKeyFrame(
                    UPDATE_STRETCH | UPDATE_POS,
                    midPafButton,
                    pafStretchStart,
                    "",
                    0.3f);
                animationController.addKeyFrame(object, kf);

                kf = AnimationController::createKeyFrameCb(
                    UPDATE_POS,
                    startPafButton,
                    startPafButton,
                    "",
                    objectCleanupCb,
                    0.2f);
                animationController.addKeyFrame(object, kf);
            } else {
                auto startStretch = vec3(-50.0f, -50.0f, 0.0f);
                auto endShift = vec3(440.0f, 0.0f, 0.0f);
                auto kf = AnimationController::createKeyFrame(
                    UPDATE_STRETCH,
                    startStretch,
                    startStretch,
                    "",
                    0.3f);
                animationController.addKeyFrame(object, kf);
                kf = AnimationController::createKeyFrameCb(
                    UPDATE_POS,
                    object->getPosition() - endShift,
                    startStretch,
                    "",
                    objectCleanupCb,
                    0.2f);
                animationController.addKeyFrame(object, kf);
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
    auto textScale = 0.8f;  // Good for 720p
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
        auto box = currentGame->createUi(textBoxImage,
            startPos + currentShift,
            0.7f,
            startStretch.x,
            startStretch.y,
            gfxController.getProgramId(4).get(),
            "option" + std::to_string(currentOptionIndex));
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
            1.0f);

        auto k1 = AnimationController::createKeyFrame(
            UPDATE_POS,
            box->getPosition() + endShift,
            startPos,
            "",
            0.2f);

        auto k2 = AnimationController::createKeyFrame(
            UPDATE_STRETCH,
            startPos,
            endStretch,
            "",
            0.3f);

        animationController.addKeyFrame(box, k0);
        animationController.addKeyFrame(box, k1);
        animationController.addKeyFrame(box, k2);

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
            1.5f);

        auto t1 = AnimationController::createKeyFrameCb(
            UPDATE_TEXT,
            textPosStart,
            textPosStart,
            option,
            incrementOptCb,
            0.3f);

        animationController.addKeyFrame(textBox, t0);
        animationController.addKeyFrame(textBox, t1);
    }

    auto endPafButton = vec3(1030.0f, 400.0f, 0.0f);
    auto midPafButton = vec3(1130.0f, 300.0f, 0.0f);
    auto startPafButton = vec3(1280.0f, 300.0f, 0.0f);
    auto pafStretchStart = vec3(-50.0f, -50.0f, 0.0f);
    auto pafStretchEnd = vec3(50.0f, 50.0f, 0.0f);
    auto pafBox = currentGame->createUi(textBoxImage,
        startPafButton,
        0.7f,
        pafStretchStart.x,
        pafStretchStart.y,
        gfxController.getProgramId(4).get(),
        "option" + std::to_string(currentOptionIndex));
    gamein->gameCamera->addSceneObject(pafBox);
    uiElements.push_back(pafBox);

    // Create animations for each text box
    auto k0 = AnimationController::createKeyFrame(
        UPDATE_NONE,
        startPos,
        startPos,
        "",
        1.0f);

    auto k1 = AnimationController::createKeyFrame(
        UPDATE_POS,
        midPafButton,
        startPos,
        "",
        0.2f);

    auto k2 = AnimationController::createKeyFrame(
        UPDATE_STRETCH | UPDATE_POS,
        endPafButton,
        pafStretchEnd,
        "",
        0.3f);

    animationController.addKeyFrame(pafBox, k0);
    animationController.addKeyFrame(pafBox, k1);
    animationController.addKeyFrame(pafBox, k2);
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
        1.5f);

    auto t1 = AnimationController::createKeyFrameCb(
        UPDATE_TEXT,
        textPosStart,
        textPosStart,
        "  PHONE\nA FRIEND",
        incrementOptCb,
        0.3f);

    animationController.addKeyFrame(textBox, t0);
    animationController.addKeyFrame(textBox, t1);
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

bool gameTimer(double time) {
    static double waitTime;
    auto result = false;
    waitTime += deltaTime;
    if (waitTime > time) {
        // Reset the timer and return true
        waitTime = 0.0;
        result = true;
    }
    return result;
}

bool doneSpeaking() {
    auto res = wordsSpoken == wordCount;
    // Check if speaking has completed
    if (res) {
        wordCount = 0;
        wordsSpoken = 0;
    }
    return res;
}

// Return true if the answer is correct for the question
bool checkAnswer(int currentQuestion, string answer) {
    assert(currentQuestion < GAME_QUESTION_SIZE);
    return gameQuestions[currentQuestion].answer.compare(answer) == 0;
}

bool doneHealthDisplay(int maxHealth) {
    bool result = false;
    if (maxHealth + 2 == healthShown) {
        result = true;
        healthShown = 0;
    }
    return result;
}

bool doneHealthHide(int maxHealth, GameInstance *game, vector<SceneObject *> objectCache) {
    bool result = false;
    if (maxHealth + 2 == healthHidden) {
        result = true;
        healthHidden = 0;

        // Perform object cleanup here...
        for (auto obj : objectCache) {
            game->removeSceneObject(obj->getObjectName());
        }
        // Clear vector
        objectCache.clear();
    }
    int healthHideVal = healthHidden;
    printf("Healthhidden: %d\n", healthHideVal);
    return result;
}

bool messageHidden() {
    auto res = uiElementsReset == 2;
    if (res) uiElementsReset = 0;
    return res;
}

bool playSong(string songPath, float songTime, float volume, GameInstance *game) {
    // Load the song and start playing it
    static bool isPlaying;
    static int songChannel;
    auto songCompleted = false;
    if (isPlaying == false) {
        auto soundIndex = game->loadSound(songPath.c_str());
        songChannel = game->playSound(soundIndex, 0, volume);
        isPlaying = true;
    }
    if (gameTimer(songTime)) {
        isPlaying = false;
        game->stopSound(songChannel);
        songCompleted = true;
    }
    return songCompleted;
}

bool volumeRamp(float maxVolume, double volumeRampTime, AudioDirection direction, unsigned int soundIndex, GameInstance *game) {
    static double currentRampTime;
    auto result = false;
    currentRampTime += deltaTime;
    if (currentRampTime > volumeRampTime) {
        currentRampTime = volumeRampTime;
        result = true;
    }
    auto volFrac = currentRampTime / volumeRampTime;
    auto currentVolume = direction == AudioDirection::QUIETER ? maxVolume - maxVolume * volFrac : maxVolume * volFrac;
    game->changeVolume(soundIndex, currentVolume);
    if (result) {
        currentRampTime = 0.0f;
    }
    return result;
}

bool showcaseHandler(const GameQuestions &cq, GameInstance *game, CameraObject *renderer) {
    static bool showcaseStarted;
    auto completedShowcase = false;
    switch (cq.type) {
        case QuestionType::MUSIC:
            // Ramp the volume before starting the showcase
            if (!showcaseStarted) {
                showcaseStarted = volumeRamp(BG_VOLUME, BG_RAMP_SECONDS, AudioDirection::QUIETER, 0, game);
            } else {
                completedShowcase = playSong(cq.mediaData, cq.showcaseTime, cq.volume, game);
            }
            // Do some minor showcase cleanup
            if (completedShowcase) {
                // Restore background music
                game->changeVolume(0, 50.0f);
            }
            break;
        case QuestionType::TRIVIA:
            // Don't need to do anything here...
            completedShowcase = true;
            break;
        case QuestionType::IMAGE:
            if (!showcaseStarted) {
                auto endPos = vec3(500.0f, 200.0f, 0.0f);
                showcaseStarted = true;
                // Draw the image on the screen
                showcaseImage = game->createSprite(
                    cq.mediaData,
                    vec3(650.0f, 400.0f, 0.0f),
                    0.00,
                    gfxController.getProgramId(3).get(),
                    SpriteAnchor::CENTER,
                    "showcaseImage");
                // Set the showcase image to medium priority
                showcaseImage->setRenderPriority(RenderPriority::MEDIUM);
                renderer->addSceneObject(showcaseImage);
                showcaseImage->setRotation(vec3(0.0f, 0.0f, 45.0f));

                // Create keyframes manually
                auto kf0 = AnimationController::createKeyFrame(
                    UPDATE_ROTATION | UPDATE_SCALE,
                    endPos,
                    vec3(0),
                    "",
                    0.7f);
                kf0->rotation.desired = vec3(0.0f, 0.0f, 360.0f);
                kf0->scale.desired = 0.80f;

                animationController.addKeyFrame(showcaseImage, kf0);

            } else if (gameTimer(cq.showcaseTime)) {
                completedShowcase = true;
            }
            break;
        default:
            printf("showcaseHandler: Unsupported question type %d\n", cq.type);
            break;
    }
    // Reset flags on completion
    if (completedShowcase) {
        showcaseStarted = false;
    }
    return completedShowcase;
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
    string currentPhrase;

    /**
     * Types of QUESTIONS we can be asked:
     * * Name that tune.
     * * General trivia question.
     * * Name that image.
     * * Minigame
     */
    GameLogicInfo game;
    game.currentGame = currentGame;
    int selectionHandlerResult;
    vector<SceneObject *> uiObjects;
    queue<SceneObject *>chatObjectCache;
    vector<SceneObject *> healthCache;
    int currentQuestion = 4;
    string answer = "";
    // showcaseImage cleanup can be run in callback
    auto showcaseImageCleanupCb = [&gamein]() {
        gamein->currentGame->removeSceneObject(showcaseImage->getObjectName());
        showcaseImage = nullptr;
    };
    KeyFrame *tempKf;
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
                printf("game::mainLoop: FPS: %f\n", 1.0 / sum);
                fps_counter->setMessage("FPS: " + to_string(static_cast<int>(1.0 / sum)));
            }
        }

        /**
         * Game logic loop
         */

        switch (gameState) {
            case WAITING:
                // If nothing is happening (usually waiting for wheel input)
                chatObjectCache = showMessage(gameQuestions[currentQuestion].question,
                    gamein->gameCamera, gamein->currentGame);
                gameState = CHATTING;
                break;
            case CHATTING:
                // Wait for the chatting to end for the showcase step
                if (doneSpeaking()) gameState = SHOWCASE;
                break;
            case SHOWCASE:
                if (showcaseHandler(gameQuestions[currentQuestion], gamein->currentGame, gamein->gameCamera)) {
                    uiObjects = drawOptions(gameQuestions[currentQuestion].getOptions(), gamein);
                    gameState = ANSWERING;
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
                            answer = gameQuestions[currentQuestion].options[selectionHandlerResult];
                        }
                        printf("mainLoop: User answered %s\n", answer.c_str());
                    }
                }
                break;
            case CONFIRMING:
                // Wait for grunty dialoge to dismiss
                if (!messageHidden()) break;
                if (checkAnswer(currentQuestion, answer)) {
                    chatObjectCache = showMessage(gameQuestions[currentQuestion].correctResponse,
                        gamein->gameCamera, gamein->currentGame);
                } else {
                    chatObjectCache = showMessage(gameQuestions[currentQuestion].wrongResponse,
                        gamein->gameCamera, gamein->currentGame);
                }
                // Remove the showcase image at this time - we shouldn't need any special protections here because
                // the image is pretty low risk...
                if (showcaseImage != nullptr) {
                    // Can probably use constants for transformations and timing later
                    tempKf = AnimationController::createKeyFrameCb(
                        UPDATE_ROTATION | UPDATE_SCALE,
                        vec3(0),
                        vec3(0),
                        "",
                        showcaseImageCleanupCb,
                        0.7f);
                    tempKf->scale.desired = 0.0f;
                    tempKf->rotation.desired = vec3(0.0f);
                    animationController.addKeyFrame(showcaseImage, tempKf);
                }
                // Also need to show the health bar for each team
                healthCache = showTeamHealth(1, gamein->gameCamera, gamein->currentGame);
                gameState = CONFIRM_CHAT;
                break;
            case CONFIRM_CHAT:
                if (doneSpeaking() && doneHealthDisplay(MAX_HEALTH)) {
                    // Sleep for a few seconds -> make a function for that
                    printf("Finished speaking final phrase\n");
                }
                if (wordCount == 0) {
                    if (gameTimer(1.0f)) {
                        // Hurt the team if the answer is wrong
                        if (!checkAnswer(currentQuestion, answer)) {
                            playRandomHurtSound(gamein->currentGame);
                            // Damage the current team
                            teamStats[0].teamHealth--;
                            // Update the health indicator
                            updateHealthIndicator(healthCache, teamStats[0].teamHealth);
                        }
                        gameState = DAMAGE_CHECK;
                    }
                }
                break;
            case DAMAGE_CHECK:
                if (gameTimer(2.0f)) {
                    // Sleep for 2 seconds before hiding message
                    hideMessage(chatObjectCache, gamein->currentGame);
                    gameState = HEALTH_HIDE;
                    hideTeamHealth(healthCache);
                }
                break;
            case HEALTH_HIDE:
                // We can't directly AND the wait calls, so we add an extra state
                if (doneHealthHide(MAX_HEALTH, gamein->currentGame, healthCache)) {
                    gameState = QUESTION_CLEANUP;
                }
                break;
            case QUESTION_CLEANUP:
                if (messageHidden()) {
                    // GO back to waiting, choose next question
                    currentQuestion++;
                    gameState = WAITING;
                }
                break;
            default:
                printf("mainLoop: ERROR CASE\n");
                assert(false);
                break;
        }
    }
    return 0;
}

