/**
 * @file AnimationController.hpp
 * @author Christian Galvez
 * @brief Global animation controller for SceneObjects
 * @version 0.1
 * @date 2024-10-20
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once
#include <map>
#include <queue>
#include <cassert>
#include <string>
#include <functional>
#include <mutex> // NOLINT
#include <SceneObject.hpp>

// Update return values
#define UPDATE_NOT_COMPLETE 0
#define POSITION_MET 1
#define STRETCH_MET 2
#define TEXT_MET 4
#define TIME_MET 8
#define ROTATION_MET 16
#define SCALE_MET 32

// Update Types
#define UPDATE_NONE 0
#define UPDATE_POS 1
#define UPDATE_STRETCH 2
#define UPDATE_TEXT 4
#define UPDATE_TIME 8
#define UPDATE_ROTATION 16
#define UPDATE_SCALE 32

// MISC
#define CAP_POS 1
#define CAP_NEG 2
#define NUM_AXIS 3
#define UPDATE_TYPES 7
#define ANIMATION_COMPLETE_CB std::function<void(void)> callback

extern double deltaTime;

template <typename T>
class AnimationData {
 public:
    T desired;
    T original;
};

template <typename T>
class UpdateData {
 public:
    inline UpdateData(T v, bool complete) : updatedValue_ { v }, updateComplete_ { complete } {}
    T updatedValue_;
    bool updateComplete_;
};

typedef struct KeyFrame {
    AnimationData<vec3> pos;
    AnimationData<vec3> stretch;
    AnimationData<string> text;
    AnimationData<vec3> rotation;
    AnimationData<float> scale;
    float targetTime;
    float currentTime = 0.0f;
    int type;
    ANIMATION_COMPLETE_CB;
    bool hasCb;
    bool isNew = true;
} KeyFrame;

typedef struct KeyFrames {
    std::queue<KeyFrame *> kQueue;
    SceneObject *target;
} KeyFrames;

class AnimationController {
 public:
    int addKeyFrame(SceneObject *target, KeyFrame *keyFrame);
    void update();
    int updatePosition(SceneObject *target, KeyFrame *keyFrame);
    int updateRotation(SceneObject *target, KeyFrame *keyFrame);
    int updateScale(SceneObject *target, KeyFrame *keyFrame);
    int updateStretch(SceneObject *target, KeyFrame *keyFrame);
    int updateText(SceneObject *target, KeyFrame *keyFrame);
    int updateTime(SceneObject *target, KeyFrame *keyFrame);
    static KeyFrame *createKeyFrameCb(int type, ANIMATION_COMPLETE_CB, float time);
    static KeyFrame *createKeyFrame(int type, float time);
    static bool cap(float *cur, float target, float dv);
    UpdateData<vec3> updateVector(vec3 original, vec3 desired, vec3 current, KeyFrame *keyFrame);
    UpdateData<string> updateString(string original, string desired, string current, KeyFrame *keyFrame);
    UpdateData<float> updateFloat(float original, float desired, float current, KeyFrame *keyFrame);
 private:
    map<string, KeyFrames> keyFrameStore_;
    std::mutex controllerLock_;
};
