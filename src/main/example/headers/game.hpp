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
#include <shaderLoader.hpp>
#include <ModelImport.hpp>
#include <GameObject.hpp>
#include <gameInstance.hpp>
#include <config.hpp>
#include <inputMonitor.hpp>
#include <physics.hpp>
#include <common.hpp>

/*
gameInfo struct currently used to temporarily send camera and character data
to a function to handle input.
*/
typedef struct gameInfo {
    bool *isDone;
    CameraObject *gameCamera;
    GameInstance *currentGame;
} gameInfo;
