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
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
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

const float BG_VOLUME = 50.0f;
const float BG_RAMP_SECONDS = 2.0f;
const int MAX_HEALTH = 3;

#define TEAM_COUNT 3

TeamStats teamStats[TEAM_COUNT] = {
    {0, 1, "src/resources/images/Team 1.png"},  // 0 = Team 1
    {0, 1, "src/resources/images/Team 2.png"},  // 1 = Team 2
    {MAX_HEALTH, 1, "src/resources/images/Team 3.png"}   // 2 = Team 3
};

#include <GameQuestions.hpp>

string pafMessage = "You want to phone a friend? Go ahead, blindly trust your end! You have twenty seconds to discuss, time is ticking, don't make a fuss!";
int pafTime = 20.0f;
TextObject *fps_counter;
TextObject *collDebugText;
TextObject *pressUText;
// This is kind of bad, but I am going to store showcase images as a global
SpriteObject *showcaseImage = nullptr;
std::atomic<unsigned int> optionsReady(0);
std::atomic<int> wordsSpoken(0);
std::atomic<int> uiElementsReset(0);
std::atomic<int> healthShown(0);
std::atomic<int> healthHidden(0);
std::atomic<unsigned int> pafShown(0);
std::atomic<unsigned int> pafHidden(0);
int wordCount;
double deltaTime = 0.0f;
int WORDS_PER_LINE = 8;  // Arbitrary
bool hasActiveController = false;
SDL_GameController *gameController1 = NULL;
#ifdef GFX_EMBEDDED
OpenGlEsGfxController gfxController = OpenGlEsGfxController();
#else
OpenGlGfxController gfxController = OpenGlGfxController();
#endif
AnimationController animationController;

int runtime(GameInstance *currentGame);
int mainLoop(gameInfo *gamein);
void configureJoySticks();

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

    unsigned int random_number = dis(gen);
    assert(random_number >= 0 && random_number < hurtSounds.size());
    // Play the hurt sound determined with the random number
    currentGame->playSound(hurtSounds.at(random_number).c_str(), 128);
}

vector<string> getLocalIPv4Address() {
    struct ifaddrs *interfaces = nullptr;
    struct ifaddrs *ifa = nullptr;
    void *tempAddrPtr = nullptr;
    char addressBuffer[INET_ADDRSTRLEN];
    vector<string> interfacesAdds;

    // Get the list of network interfaces
    if (getifaddrs(&interfaces) == -1) {
        perror("getifaddrs");
        return interfacesAdds;
    }
    interfacesAdds.push_back("Please connect a controller");
    // Iterate over the network interfaces
    for (ifa = interfaces; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;

        // Check if the address is IPv4
        if (ifa->ifa_addr->sa_family == AF_INET) {
            // Get the address
            tempAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, tempAddrPtr, addressBuffer, INET_ADDRSTRLEN);

            // Print the address
            std::cout << "Interface: " << ifa->ifa_name << " | Address: " << addressBuffer << std::endl;
            interfacesAdds.push_back(string(ifa->ifa_name) + ": " + string(addressBuffer));
        }
    }

    // Free the memory allocated for the network interfaces list
    freeifaddrs(interfaces);

    return interfacesAdds;
}

vector<SceneObject *> showTeamHealth(unsigned int teamNumber, CameraObject *renderer, GameInstance *currentGame, int shift=0) {
    printf("showTeamHealth: Entry - teamNumber %u\n", teamNumber);
    float verticalShift = 150.0f * shift;
    float hcDisplacement = 60.0f;
    float startingX = -90.0f;
    float leftEndX = 500.0f;
    vector<SceneObject *> objectCache;

    float endStretch = 20.0f + (MAX_HEALTH * hcDisplacement);
    float startStretch = -50.0f;
    float endBox = 355.0f;
    vec3 boxPos = vec3(-140.0f, 465.0f + verticalShift, 0.0f);
    vec3 boxStretch = vec3(startStretch, startStretch, 0.0f);

    // Grab the current health for this team
    assert(teamNumber >= 0);
    assert(teamNumber < (sizeof(teamStats) / sizeof(TeamStats)));

    auto currentHealth = teamStats[teamNumber].teamHealth;

    // Team image positions
    vec3 teamEndPos = vec3(380.0f, 450.0f + verticalShift, 0.0f);
    vec3 teamStartPos = vec3(-150.0f, 450.0f + verticalShift, 0.0f);

    // Create textbox for healthbar backdrop
    auto box = currentGame->createUi(textBoxImage,
        boxPos,
        0.7f,
        startStretch,
        -50.0f,
        gfxController.getProgramId(4).get(),
        ObjectAnchor::BOTTOM_LEFT,
    "healthbg" + std::to_string(teamNumber));

    renderer->addSceneObject(box);

    auto teamSprite = currentGame->createSprite(teamStats[teamNumber].imagePath,
        teamStartPos,
        0.40f,
        gfxController.getProgramId(3).get(),
        ObjectAnchor::BOTTOM_LEFT,
        "team" + std::to_string(teamNumber));

    // Programatically create health bar icons
    for (int i = 0; i < MAX_HEALTH; ++i) {
        auto honeyComb = currentGame->createSprite(
            honeycombFull,
            vec3(startingX, 440.0f + verticalShift, 0.0f),
            0.45f,
            gfxController.getProgramId(3).get(),
            ObjectAnchor::BOTTOM_LEFT,
            "hc" + std::to_string(i) + " " + std::to_string(teamNumber));

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
    renderer->addSceneObject(teamSprite);

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
        teamEndPos,
        vec3(0),
        "",
        cb,
        leftReachTime);

    // Create a kf1 based on the deltaSlideTime and displacement
    // Add the keyframes to the animation controller
    animationController.addKeyFrame(teamSprite, kf0);

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
    objectCache.push_back(teamSprite);

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

    float endStretch = -50.0f;
    float endBox = -140.0f;
    vec3 boxStretch = vec3(endStretch, endStretch, 0.0f);

    float leftReachTime = 0.7f;
    float deltaSlideTime = 0.1f;
    float maxDeltaSlide = 0.0f;

    // Team image positions
    float teamEnd = -150.0f;
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

    auto boxPos = box->getPosition();
    boxPos.x = endBox;

    // Reverse the background health image animation
    // Shrink the text box and hide it
    auto kfbox = AnimationController::createKeyFrame(
        UPDATE_STRETCH,
        vec3(0),
        boxStretch,
        "",
        maxDeltaSlide);

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
    auto teamPos = teamImage->getPosition();
    teamPos.x = teamEnd;

    // Hide the team image with the same timing as everything else
    auto teamKf = AnimationController::createKeyFrame(
        UPDATE_NONE,
        vec3(0),
        vec3(0),
        "",
        maxDeltaSlide);

    auto teamKf1 = AnimationController::createKeyFrameCb(
        UPDATE_POS,
        teamPos,
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
    // Need to account for HIGH DPI modes (250.0f for High DPI (mac), 125.0f for normal DPI (everything else))
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
        ObjectAnchor::BOTTOM_LEFT,
    "grunty");

    auto box = currentGame->createUi(textBoxImage,
        vec3(-220.0f, 150.0f, 0.0f),
        0.7f,
        -50.0f,
        -50.0f,
        gfxController.getProgramId(4).get(),
        ObjectAnchor::BOTTOM_LEFT,
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
        ObjectAnchor::BOTTOM_LEFT,
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

vector<SceneObject *> drawOptions(vector<string> options, int team, gameInfo *gamein) {
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
            ObjectAnchor::BOTTOM_LEFT,
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

    // Return early if team does not have paf
    if (!teamStats[team].paf) return uiElements;

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
        ObjectAnchor::BOTTOM_LEFT,
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

int selectionHandler(GameLogicInfo *game, vector<SceneObject *> optionCache, int team) {
    int result = -1;
    bool hasPaf = teamStats[team].paf;
    unsigned int numOptions = optionCache.size() / 2;
    unsigned int numOptionsExcludePaf = numOptions - (hasPaf ? 1 : 0);
    assert(numOptions > 0);
    // Input
    auto dp_up = game->currentGame->getKeystate()[SDL_SCANCODE_W];
    auto dp_down = game->currentGame->getKeystate()[SDL_SCANCODE_S];
    auto dp_left = game->currentGame->getKeystate()[SDL_SCANCODE_A];
    auto dp_right = game->currentGame->getKeystate()[SDL_SCANCODE_D];
    auto dp_select = game->currentGame->getKeystate()[SDL_SCANCODE_RETURN];

    // updateHasActiveController if needed
    configureJoySticks();

    // Add controller inputs
    if (hasActiveController) {
        dp_up = dp_up || SDL_GameControllerGetButton(gameController1,
            SDL_CONTROLLER_BUTTON_DPAD_UP);
        dp_down = dp_down || SDL_GameControllerGetButton(gameController1,
            SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        dp_left = dp_left || SDL_GameControllerGetButton(gameController1,
            SDL_CONTROLLER_BUTTON_DPAD_LEFT);
        dp_right = dp_right || SDL_GameControllerGetButton(gameController1,
            SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
        dp_select = dp_select || SDL_GameControllerGetButton(gameController1,
            SDL_CONTROLLER_BUTTON_A);
    }
    // numOptions = 5 w/ paf
    if (dp_up && debounceCheck(game)) {
        // "up" action
        // Change the selected option up to numOptionsExcludePaf - 1 (size to index)
        if (game->currentOption < numOptionsExcludePaf - 1) {
            game->currentOption++;
            game->currentGame->playSound(5, 0, 50);
        }
        game->currentDebounce = 0.0f;
    } else if (dp_down && debounceCheck(game)) {
        // "down" action
        // Don't go below index 0
        if (game->currentOption > 0) {
            game->currentOption--;
            game->currentGame->playSound(5, 0, 50);
        }
        game->currentDebounce = 0.0f;
    } else if (dp_right && debounceCheck(game)) {
        // "right" action
        // Switch to option 4
        if (hasPaf && game->currentOption != numOptionsExcludePaf) {
            game->prevOption = game->currentOption;
            game->currentOption = numOptionsExcludePaf;
            game->currentGame->playSound(5, 0, 50);
        }
        game->currentDebounce = 0.0f;
    } else if (dp_left && debounceCheck(game)) {
        // "left" action
        // Change the selected option back from "PAF"
        if (hasPaf && game->currentOption == numOptionsExcludePaf) {
            game->currentOption = game->prevOption;
            game->currentGame->playSound(5, 0, 50);
        }
        game->currentDebounce = 0.0f;
    } else if (dp_select && debounceCheck(game)) {
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
        wordsSpoken = 0;
    }
    return res;
}

// Return true if the answer is correct for the question
bool checkAnswer(string correctAnswer, string answer) {
    return correctAnswer.compare(answer) == 0;
}

bool doneHealthDisplay(vector<SceneObject *> healthCache) {
    bool result = false;
    int elemCount = healthCache.size();
    if (elemCount == healthShown) {
        result = true;
        healthShown = 0;
    }
    return result;
}

bool doneHealthHide(GameInstance *game, vector<SceneObject *> healthCache) {
    bool result = false;
    int elemCount = healthCache.size();
    if (elemCount == healthHidden) {
        result = true;
        healthHidden = 0;

        // Perform object cleanup here...
        for (auto obj : healthCache) {
            game->removeSceneObject(obj->getObjectName());
        }
        // Clear vector
        healthCache.clear();
    }
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

bool optionsDrawn(vector<SceneObject *> objectCache) {
    // Options = ui cache size / 2 because each option is a text box and text object
    auto targetOptions = objectCache.size() / 2;
    assert(targetOptions > 0);  // We shouldn't ever have zero options...
    auto result = false;
    if (optionsReady == targetOptions) {
        optionsReady = 0;
        result = true;
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
                    ObjectAnchor::CENTER,
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

bool pafTimerShown(vector<SceneObject *> pafCache) {
    auto result = false;
    if (pafShown == pafCache.size()) {
        pafShown = 0;
        result = true;
    }
    return result;
}

bool pafTimerHidden(vector<SceneObject *> pafCache, GameInstance *game) {
    auto result = false;
    if (pafHidden == pafCache.size()) {
        pafHidden = 0;
        result = true;

        // Perform cleanup
        for (auto obj : pafCache) {
            game->removeSceneObject(obj->getObjectName());
        }
    }
    return result;
}

void hidePafTimer(vector<SceneObject *> pafCache, GameInstance *game) {
    float hideTime = 1.0f;
    auto startTextPos = vec3(-90.0f, 420.0f, 0.0f);
    auto startBoxPos = vec3(-140.0f, 500.0f, 0.0f);

    auto cb = []() {
        pafHidden++;
    };

    for (auto obj : pafCache) {
        vec3 pos = vec3(0);
        if (obj->type() == UI_OBJECT) pos = startBoxPos;
        if (obj->type() == TEXT_OBJECT) {
            pos = startTextPos;
            // Set the timer text to zero
            auto cobj = static_cast<TextObject *>(obj);
            cobj->setMessage("0");
        }
        auto kf = AnimationController::createKeyFrameCb(
            UPDATE_POS,
            pos,
            vec3(0),
            "",
            cb,
            hideTime);

        animationController.addKeyFrame(obj, kf);
    }
}

vector<SceneObject *> createPafTimer(GameInstance *game, CameraObject *renderer) {
    auto textScale = 1.0f;
    auto transitionTime = 1.0f;
    auto fontPath = "src/resources/fonts/Comic Sans MS.ttf";
    auto textProgramId = gfxController.getProgramId(2).get();
    vector<SceneObject *> pafCache;

    auto cb = []() {
        pafShown++;
    };

    auto endTextPos = vec3(630.0f, 420.0f, 0.0f);
    auto endBoxPos = vec3(580.0f, 500.0f, 0.0f);

    auto startTextPos = vec3(-50.0f, 420.0f, 0.0f);
    auto startBoxPos = vec3(-100.0f, 500.0f, 0.0f);
    // Create the text and UI objects on screen
    auto pafText = game->createText(
        std::to_string(pafTime),
        startTextPos,
        textScale,
        fontPath,
        textProgramId,
        "pafTimer");
    pafText->setRenderPriority(RenderPriority::HIGH);

    auto pafBox = game->createUi(textBoxImage,
        startBoxPos,
        0.7f,
        -50.0f,
        -50.0f,
        gfxController.getProgramId(4).get(),
        ObjectAnchor::BOTTOM_LEFT,
        "pafBoxBg");
    pafBox->setRenderPriority(RenderPriority::MEDIUM);
    pafCache.push_back(pafText);
    pafCache.push_back(pafBox);
    // Add the created gameobjects to the renderer
    for (auto obj : pafCache) {
        renderer->addSceneObject(obj);
    }

    // Add animation keyframes for timer entry
    auto boxKf = AnimationController::createKeyFrameCb(
        UPDATE_POS,
        endBoxPos,
        vec3(0),
        "",
        cb,
        transitionTime);

    auto textKf = AnimationController::createKeyFrameCb(
        UPDATE_POS,
        endTextPos,
        vec3(0),
        "",
        cb,
        transitionTime);

    animationController.addKeyFrame(pafBox, boxKf);
    animationController.addKeyFrame(pafText, textKf);
    return pafCache;
}

// Hide the current team when the question is answered
void hideTeamIcon(GameInstance *currentGame, vector<SceneObject *> teamIconCache) {
    // Create keyframes to hide the team health
    auto shiftTime = 0.5f;
    auto leftShift = -160;

    // Undo the right shift
    for (auto obj : teamIconCache) {
        auto pos = obj->getPosition();
        auto objName = obj->getObjectName();
        pos.x += leftShift;

        // Delete the gameobject cache when the animation completes
        auto cb = [currentGame, objName]() {
            currentGame->removeSceneObject(objName);
        };
        // Get the current object position
        auto kf = AnimationController::createKeyFrameCb(
            UPDATE_POS,
            pos,
            vec3(0),
            "",
            cb,
            shiftTime);

        animationController.addKeyFrame(obj, kf);
    }
}

// Show the current team after the team health is displayed
vector<SceneObject *> showTeamIcon(GameInstance *currentGame, CameraObject *renderer, int team) {
    auto rightShift = 160;
    auto shiftTime = 0.3f;
    auto sleepTime = 1.0f;
    auto boxPos = vec3(0, 300, 0);
    //boxPos.x += rightShift;
    auto startStretch = vec3(0, 0, 0);
    auto teamPos = vec3(-63, 365, 0);
    //teamPos.x += rightShift;
    auto bgScale = 0.5f;

    vector<SceneObject *> teamIconCache;

    // Create textbox for healthbar backdrop
    auto teambg = currentGame->createUi(textBoxImage,
        boxPos,
        bgScale,
        startStretch.x,
        startStretch.y,
        gfxController.getProgramId(4).get(),
        ObjectAnchor::CENTER,
        "teambg" + std::to_string(team));

    // Grab the current team image (small)
    auto teamSprite = currentGame->createSprite(teamStats[team].imagePath,
        teamPos,
        0.40f,
        gfxController.getProgramId(3).get(),
        ObjectAnchor::CENTER,
        "team" + std::to_string(team));

    teamIconCache.push_back(teambg);
    teamIconCache.push_back(teamSprite);

    // Add all of the objects to the renderer
    for (auto obj : teamIconCache) {
        // Create shift keyframe for each object
        auto targetPos = obj->getPosition();
        targetPos.x += rightShift;

        auto sleep_kf = AnimationController::createKeyFrame(
            UPDATE_NONE,
            vec3(0),
            vec3(0),
            "",
            sleepTime);

        auto kf = AnimationController::createKeyFrame(
            UPDATE_POS,
            targetPos,
            vec3(0),
            "",
            shiftTime);

        animationController.addKeyFrame(obj, sleep_kf);
        animationController.addKeyFrame(obj, kf);

        renderer->addSceneObject(obj);
    }

    return teamIconCache;
}
vector<unsigned int> getTeams() {
    vector<unsigned int> eligibleTeams;
    // Check which teams are gone
    for (unsigned int i = 0; i < TEAM_COUNT; ++i) {
        // If a team is dead, don't choose it
        if (teamStats[i].teamHealth == 0) {
            // Ignore this team
            printf("Team %d is dead, not selecting...\n", i);
        } else {
            eligibleTeams.push_back(i);
        }
    }
    return eligibleTeams;
}

unsigned int getNextTeam(unsigned int currentTeam) {
    // Get the teams that are still alive
    auto teams = getTeams();
    // Add a guard to return early if we only have one team left
    if (teams.size() == 1) {
        return teams.front();
    }
    // currentTeam is either 0, 1 or 2
    auto desiredNextTeam = currentTeam + 1 < TEAM_COUNT ? currentTeam + 1 : 0;
    auto desiredTeamExists = false;
    // Check if the desired team exists
    for (auto team : teams) {
        if (team == desiredNextTeam) {
            desiredTeamExists = true;
            break;
        }
    }
    // If the desired team doesn't exist, then grab the one after it
    if (!desiredTeamExists) {
        auto firstTeam = teams.front();
        auto foundNextTeam = false;
        for (auto team : teams) {
            if (team > currentTeam) {
                foundNextTeam = true;
                desiredNextTeam = team;
                break;
            }
        }
        // If no greater team exist, then wrap around using first team...
        if (!foundNextTeam)
            desiredNextTeam = firstTeam;
    }
    // Make sure we're not returning the original team queried
    assert(currentTeam != desiredNextTeam);

    return desiredNextTeam;
}

unsigned int numberOfTeamsRemaining() {
    auto teams = getTeams();
    return teams.size();
}

void configureJoySticks() {
    static int wasConfigured;
    static int lastJoyNum;
    auto numJoySticks = 0;

    numJoySticks = SDL_NumJoysticks();
    // If the configuration has changed...
    if (wasConfigured && lastJoyNum != numJoySticks) {
        wasConfigured = 0;
    }
    // Then we re-configure the controller (to handle disconnects)
    if (!wasConfigured) {
        // Attempt to configure joysticks here
        for (int i = 0; i < numJoySticks; i++) {
            if (SDL_IsGameController(i)) {
                printf("NumJoysticks: %d, curController: %d\n", numJoySticks, i);
                gameController1 = SDL_GameControllerOpen(i);
                if (gameController1 != 0) {
                    hasActiveController = true;
                    wasConfigured = 1;
                    lastJoyNum = numJoySticks;
                }
            }
        }
    }
}

/*
 (int) mainLoop starts rendering objects in the current GameInstance to the
 main SDL window. The methods called from the currentGame object render parts
 of the scene.

 (int) mainLoop returns 0 when closed successfully. When an error occurs and the
 mainLoop closes prematurely, an error code is returned.
*/
int mainLoop(gameInfo* gamein) {
    auto gameState = BOOTUP_SCREEN;
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
    vector<SceneObject *> tempCache;
    vector<SceneObject *> teamIconCache;
    GameQuestions currentQuestion;
    string answer = "";
    // showcaseImage cleanup can be run in callback
    auto showcaseImageCleanupCb = [&gamein]() {
        gamein->currentGame->removeSceneObject(showcaseImage->getObjectName());
        showcaseImage = nullptr;
    };
    KeyFrame *tempKf;

    // Text dimming for option selection
    auto dimColor = vec3(0.3f, 0.3f, 0.3f);
    auto brightColor = vec3(1.0f);

    // PAF timer objects
    vector<SceneObject *> pafCache;
    float remainingPafTime = 0.0f;

    // Misc counters
    int stageCounter = 0;
    int configured = 0;
    int pafSkip = 0;

    // Team tracking
    unsigned int currentTeam = 0;

    // Configs
    vector<string> addrs;
    vector<SceneObject *> addrCache;

    // Now... Attempt to load ALL questions into a vector
    vector<GameQuestions> questions;
    for (int i = 0; i < GAME_QUESTION_SIZE; ++i) {
        // Shuffle options before inserting
        auto curQues = gameQuestions[i];
        auto seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(curQues.options.begin(), curQues.options.end(), std::default_random_engine(seed));
        questions.push_back(curQues);
    }

    // Now shuffle all of the questions around...
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(questions.begin(), questions.end(), std::default_random_engine(seed));
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
            case BOOTUP_SCREEN:
                if (configured == 0 && hasActiveController) {
                    gameState = BEGIN_ROUND;
                    break;
                }
                if (!configured) {
                    // Set the background to be black
                    auto bd = currentGame->getSceneObject("Backdrop");
                    assert(bd != nullptr);
                    // Add a dark tint to the backdrop
                    auto cbd = (SpriteObject *)bd;
                    cbd->setTint(vec3(-0.5f));

                    configured = 1;
                    addrs = getLocalIPv4Address();
                    const auto topLine = vec3(380.0f, 500.0f, 0.0f);
                    auto downShift = 60.0f;
                    auto textScale = 0.8f;  // Good for 720p
                    auto fontPath = "src/resources/fonts/Comic Sans MS.ttf";
                    auto textProgramId = gfxController.getProgramId(2).get();
                    auto textBoxId = 0;
                    // Loop through addrs and draw them on screen
                    for (auto addr : addrs) {
                        auto pos = topLine;
                        pos.y -= (downShift * textBoxId);
                        if (textBoxId > 0) pos.x += downShift * 2;
                        auto text = currentGame->createText(addr,
                            pos,
                            textScale,
                            fontPath,
                            textProgramId,
                            "AddrText" + std::to_string(textBoxId++));
                        // Add the text to the renderer
                        gamein->gameCamera->addSceneObject(text);

                        // Add text to the cache
                        addrCache.push_back(text);
                    }
                } else {
                    // Check the number of controllers connected
                    // Initialize new controller here
                    configureJoySticks();
                }
                if (hasActiveController) {
                    // Do some cleanup here
                    for (auto obj : addrCache) {
                        currentGame->removeSceneObject(obj->getObjectName());
                    }
                    gameState = BEGIN_ROUND;

                    // Undo tint
                    // Set the background to be black
                    auto bd = currentGame->getSceneObject("Backdrop");
                    assert(bd != nullptr);
                    // Add a dark tint to the backdrop
                    auto cbd = (SpriteObject *)bd;
                    cbd->setTint(vec3(0.0f));
                }
                break;
            case BEGIN_ROUND:
                // Show all team health bars
                healthCache.clear();
                // Show each health bar
                for (int i = 0; i < TEAM_COUNT; ++i) {
                    tempCache = showTeamHealth(i, gamein->gameCamera, gamein->currentGame, i - 1);
                    // Insert the tempCache elements into the healthcache
                    healthCache.insert(healthCache.end(), tempCache.begin(), tempCache.end());
                }

                // Say some introductory message
                chatObjectCache = showMessage("Game summary text placeholder.", gamein->gameCamera, gamein->currentGame);
                gameState = BEGIN_ROUND_UI_WAIT;
                break;

            case BEGIN_ROUND_UI_WAIT:
                // Wait on the message and all health elements to be drawn
                if (doneSpeaking()) {
                    stageCounter++;
                }
                if (doneHealthDisplay(healthCache)) {
                    stageCounter++;
                }
                if (stageCounter == 2 && gameTimer(1.5f)) {
                    // Show the health elements for 1.5 seconds
                    gameState = BEGIN_ROUND_UI_CLOSE;
                    printf("HALT");
                    stageCounter = 0;

                    // Hide all elements
                    hideMessage(chatObjectCache, gamein->currentGame);
                    for (int i = 0; i < TEAM_COUNT; ++i) {
                        // Create subvectors for each team
                        auto vecSize = healthCache.size() / TEAM_COUNT;
                        auto subStart = healthCache.begin() + (vecSize * i);
                        auto subEnd = healthCache.begin() + (vecSize * (i + 1));
                        auto subVec = vector<SceneObject *> (subStart, subEnd);

                        // Remove health bar by subvector
                        hideTeamHealth(subVec);
                    }
                }
                break;
            case BEGIN_ROUND_UI_CLOSE:
                if (doneHealthHide(gamein->currentGame, healthCache)) {
                    stageCounter++;
                }
                if (messageHidden()) {
                    stageCounter++;
                }
                if (stageCounter == 2) {
                    // Go to waiting phase
                    gameState = WAITING;
                    stageCounter = 0;
                }
                break;
            case WAITING:
                // Keep the current question for a paf skip
                if (!pafSkip) {
                    // Pop the front question off to ask!
                    currentQuestion = questions.front();
                    // Remove the front question
                    questions.erase(questions.begin());
                } else {
                    pafSkip = 0;
                }
                // If nothing is happening (usually waiting for wheel input)
                chatObjectCache = showMessage(currentQuestion.question,
                    gamein->gameCamera, gamein->currentGame);
                teamIconCache = showTeamIcon(gamein->currentGame, gamein->gameCamera, currentTeam);
                gameState = CHATTING;
                break;
            case CHATTING:
                // Wait for the chatting to end for the showcase step
                if (doneSpeaking()) gameState = SHOWCASE;
                break;
            case SHOWCASE:
                if (showcaseHandler(currentQuestion, gamein->currentGame, gamein->gameCamera)) {
                    uiObjects = drawOptions(currentQuestion.options, currentTeam, gamein);
                    gameState = DISPLAY_OPTIONS;
                }
                break;
            case DISPLAY_OPTIONS:
                if (optionsDrawn(uiObjects)) gameState = ANSWERING;
                break;
            case ANSWERING:
                // "dim" unselected options
                selectionHandlerResult = selectionHandler(&game, uiObjects, currentTeam);
                // Dim unselected options, highlight selected option
                for (unsigned int i = 0; i < (uiObjects.size() / 2); ++i) {
                    auto obj = currentGame->getSceneObject(string("OptionText") + std::to_string(i));
                    assert(obj != nullptr);
                    // Cast the object to TextObject
                    assert(obj->type() == ObjectType::TEXT_OBJECT);
                    auto cobj = static_cast<TextObject *>(obj);
                    if (game.currentOption == i) {
                        cobj->setColor(brightColor);
                    } else {
                        cobj->setColor(dimColor);
                    }
                }
                if (selectionHandlerResult != -1) {
                    // Perform selection operation
                    gameState = CONFIRMING;
                    hideOptions(uiObjects, &game);
                    hideMessage(chatObjectCache, game.currentGame);
                    hideTeamIcon(gamein->currentGame, teamIconCache);
                    // The last option is PAF IF PAF is active!!!
                    if (teamStats[currentTeam].paf && selectionHandlerResult == static_cast<int>((uiObjects.size() / 2) - 1)) {
                        answer = "PHONE A FRIEND";
                        gameState = PHONE_A_FRIEND;
                        // Remove paf from teamStats
                        teamStats[currentTeam].paf = 0;
                        // Revert current selection to zero
                    } else {
                        // The answer may be shuffled, so 
                        answer = currentQuestion.options[selectionHandlerResult];
                    }
                    game.currentOption = 0;
                    printf("mainLoop: User answered %s\n", answer.c_str());
                }
                break;
            case PHONE_A_FRIEND:
                pafSkip = 1;
                // Wait for the current message to hide
                if (!messageHidden()) break;
                chatObjectCache = showMessage(pafMessage, gamein->gameCamera, gamein->currentGame);
                gameState = PAF_PRECLOCK;
                break;
            case PAF_PRECLOCK:
                if (doneSpeaking()) {
                    gameState = PAF_ENTRY;

                    // Configure the paf clock objects
                    pafCache = createPafTimer(gamein->currentGame, gamein->gameCamera);
                    remainingPafTime = pafTime;
                }
                break;
            case PAF_ENTRY:
                // Waits for the PAF elements to be drawn
                if (pafTimerShown(pafCache)) {
                    gameState = PAF_CLOCK;
                }
                break;
            case PAF_CLOCK:
                // Start a game timer, and show the active time on-screen
                if (gameTimer(pafTime)) {
                    gameState = PAF_EXIT;
                    hidePafTimer(pafCache, gamein->currentGame);
                    // Hide the current message
                    hideMessage(chatObjectCache, gamein->currentGame);

                } else {
                    // Update the text on the timer object
                    assert(!pafCache.empty());
                    assert(pafCache.front()->type() == ObjectType::TEXT_OBJECT);
                    auto cobj = static_cast<TextObject *>(pafCache.front());
                    // Change the message to the remaining time
                    cobj->setMessage(std::to_string(static_cast<int>(remainingPafTime) + 1));
                    // Don't subtract deltaTime until later to avoid negative numbers
                    remainingPafTime -= deltaTime;
                }
            case PAF_EXIT:
                // Wait for message to hide
                if (messageHidden()) {
                    stageCounter++;
                }
                // Wait for paf timer to hide
                if (pafTimerHidden(pafCache, gamein->currentGame)) {
                    stageCounter++;
                }

                // Reset to waiting stage if all elements hidden
                // Make stage counter global for easier use
                if (stageCounter == 2) {
                    gameState = WAITING;
                    stageCounter = 0;
                    // Remove the showcase image when we PAF...
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
                }
                break;
            case CONFIRMING:
                // Wait for grunty dialoge to dismiss
                if (!messageHidden()) break;
                if (checkAnswer(currentQuestion.answer, answer)) {
                    chatObjectCache = showMessage(currentQuestion.correctResponse,
                        gamein->gameCamera, gamein->currentGame);
                } else {
                    chatObjectCache = showMessage(currentQuestion.wrongResponse,
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
                healthCache = showTeamHealth(currentTeam, gamein->gameCamera, gamein->currentGame);
                gameState = CONFIRM_CHAT;
                break;
            case CONFIRM_CHAT:
                if (doneSpeaking()) {
                    // Sleep for a few seconds -> make a function for that
                    printf("Finished speaking final phrase\n");
                    stageCounter++;
                }
                if (doneHealthDisplay(healthCache)) {
                    stageCounter++;
                }
                if (stageCounter == 2) {
                    if (gameTimer(1.0f)) {
                        // Hurt the team if the answer is wrong
                        if (!checkAnswer(currentQuestion.answer, answer)) {
                            playRandomHurtSound(gamein->currentGame);
                            // Damage the current team
                            teamStats[currentTeam].teamHealth--;
                            // Update the health indicator
                            updateHealthIndicator(healthCache, teamStats[currentTeam].teamHealth);
                        }
                        gameState = DAMAGE_CHECK;
                        stageCounter = 0;
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
                if (doneHealthHide(gamein->currentGame, healthCache)) {
                    gameState = QUESTION_CLEANUP;
                }
                break;
            case QUESTION_CLEANUP:
                if (messageHidden()) {
                    // GO back to waiting, choose next question
                    // Check for a winner
                    auto teamsLeft = numberOfTeamsRemaining();
                    auto doNewRound = false;
                    if (teamsLeft == 1) {
                        auto teams = getTeams();
                        for (auto i : teams) {
                            tempCache = showTeamHealth(i, gamein->gameCamera, gamein->currentGame, i - 1);
                            // Insert the tempCache elements into the healthcache
                            healthCache.insert(healthCache.end(), tempCache.begin(), tempCache.end());
                        }
                        // Which team won?
                        auto winningTeam = teams.front();
                        string winningMessage = "LOL";
                        switch (winningTeam) {
                            case 0:
                              winningMessage = "Well, well, well Team 1 has won. I hope you all have had some fun!";
                              break;
                            case 1:
                              winningMessage = "Looks like team 2 has won these rounds. You really showed up all these clowns!";
                              break;
                            case 2:
                              winningMessage = "Everyone, it looks like the winner is team 3! They will be taking home the victory!";
                              break;
                            default:
                              winningMessage = "This is awkward, I must confess. This game appears to be a mess! This statement you should never see, the default case has been hit you see!";
                              break;
                        }
                        // There is a winner
                        chatObjectCache = showMessage(winningMessage, gamein->gameCamera, gamein->currentGame);
                        gameState = HEALTH_HIDE;
                        break;
                    } else if (questions.empty()) {
                        chatObjectCache = showMessage(
                            "Well this result is unexpected! More than one team is left uncontested! I'm not really sure what happens now, maybe you all can take a bow!",
                            gamein->gameCamera,
                            gamein->currentGame);
                        // get the teams still alive
                        auto teams = getTeams();
                        for (auto i : teams) {
                            tempCache = showTeamHealth(i, gamein->gameCamera, gamein->currentGame, i - 1);
                            // Insert the tempCache elements into the healthcache
                            healthCache.insert(healthCache.end(), tempCache.begin(), tempCache.end());
                        }
                        gameState = HEALTH_HIDE;
                        break;
                    } else {
                        auto lastTeam = currentTeam;
                        // If multiple teams still exist, choose the next one
                        currentTeam = getNextTeam(currentTeam);
                        doNewRound = lastTeam > currentTeam;
                    }
                    gameState = doNewRound ? BEGIN_ROUND : WAITING;
                }
                break;
            default:
                printf("mainLoop: ERROR CASE\n");
                assert(false);
                break;
        }
    }
    // Cleanup controller if needed
    SDL_GameControllerClose(gameController1);
    return 0;
}

