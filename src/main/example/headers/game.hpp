/**
 * @file game.hpp
 * @author Christian Galvez
 * @brief Header for "main" game definition file
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once
#include <vector>
#include <string>
#include <ModelImport.hpp>
#include <GameObject.hpp>
#include <GameInstance.hpp>
#include <config.hpp>
#include <inputMonitor.hpp>
#include <physics.hpp>
#include <common.hpp>

/*
gameInfo struct currently used to temporarily send camera and character data
to a function to handle input.
*/
struct gameInfo {
    bool *isDone;
    CameraObject *gameCamera;
    GameInstance *currentGame;
};

enum GameState {
    CHATTING,
    ANSWERING,
    SHOWCASE,
    WAITING,
    CONFIRMING,
    CONFIRM_CHAT,
    DAMAGE_CHECK,
    QUESTION_CLEANUP
};

enum QuestionType {
    MUSIC,
    TRIVIA,
    IMAGE,
    MINIGAME
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
    float songTimePass = 12.0f;
    float volumeRampSeconds = 2.0f;
    float currentVolumeRampSeconds = 0.0f;
    int songChannel = -1;
};

struct GameQuestions {
    string options[4];
    string question;
    QuestionType type;
    float showcaseTime;
    float volume;
    string answer;
    string correctResponse;
    string wrongResponse;
    string mediaData;
    vector<string> getOptions() { return vector<string>(options, options + 4); }
};

enum AudioDirection {
    LOUDER,
    QUIETER
};

struct TeamStats {
    int teamHealth;
    int paf;  // Phone a friend
};
