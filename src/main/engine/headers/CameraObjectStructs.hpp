/**
 * @file CameraObjectStructs.hpp
 * @author Christian Galvez
 * @brief Contains the struct used to instantiate a CameraObject
 * @version 0.1
 * @date 2023-07-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once
#include <GameObject.hpp>

/**
 * @brief Used to construct CameraObjects
 * @param objTarget(GameObject*) Object for camera to target onto.
 * @param offset(vec3) 3D vector describing position offset of Camera relative to
 * the target GameObject.
 * @param viewCameraAngle(GLfloat) Describes the field of view of the CameraObject to
 * be created.
 * @param viewAspectRatio(GLfloat) Describes the aspect ratio of the camera view.
 * This value SHOULD always be set to the current aspect ratio of the SDL
 * window, or else things will look stretched out or compressed.
 * @param viewNearClipping(GLfloat) Describes the distance for near clipping to
 * occur. This value should be as low as the graphics card allows.
 * @param viewFarClipping(GLfloat) Describes the distance for far clipping to occur.
     This value should be as high as the graphics card allows.
 */
typedef struct cameraInfo {
    GameObject *objTarget;
    vec3 offset;
    GLfloat viewCameraAngle, viewAspectRatio, viewNearClipping, viewFarClipping;
} cameraInfo;
