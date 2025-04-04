/**
 * @file inputMonitor.hpp
 * @author Alec Jackson
 * @brief Basic input monitor to control GameObjects in the scene
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once
#include <game.hpp>
#include <AnimationController.hpp>
extern AnimationController animationController;
extern double deltaTime;
void rotateShape(void *gameInfoStruct, void *target);
float convertNegToDeg(float degree);
float angleOfPoint(vec3 p1, vec3 p2);
