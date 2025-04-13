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
#include <memory>
#include <vector>
#include <SceneObject.hpp>
#include <SpriteObject.hpp>

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

enum class TrackState {
    PAUSED,
    RUNNING
};

struct AnimationTrack {
    vector<int> trackData;
    string trackName;
    int targetFps;
    inline AnimationTrack(vector<int> tD, string tN, int tF) :
        trackData { tD }, trackName { tN }, targetFps { tF } {};
};

struct AnimationTracks {
    SpriteObject *target;
    std::map<string, std::shared_ptr<AnimationTrack>> tracks;
};

struct TrackPlayback {
    TrackState state = TrackState::RUNNING;
    std::shared_ptr<AnimationTrack> track;
    float secondsPerFrame;
    float sequenceTime;
    float currentTime = 0.0f;
    int currentTrackIdx;
    SpriteObject *target;
    inline TrackPlayback(std::shared_ptr<AnimationTrack> tr, float sPF, float sT, int cTI, SpriteObject *ta) :
        track { tr }, secondsPerFrame { sPF }, sequenceTime { sT }, currentTrackIdx { cTI }, target { ta } {};
};

struct KeyFrame {
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
};

struct KeyFrames {
    std::queue<KeyFrame *> kQueue;
    SceneObject *target;
};

class AnimationController {
 public:
    int addKeyFrame(SceneObject *target, std::shared_ptr<KeyFrame> keyFrame);
    void addTrack(SpriteObject *target, string trackName, vector<int> trackData, int fps);
    void update();
    int updatePosition(SceneObject *target, KeyFrame *keyFrame);
    int updateRotation(SceneObject *target, KeyFrame *keyFrame);
    int updateScale(SceneObject *target, KeyFrame *keyFrame);
    int updateStretch(SceneObject *target, KeyFrame *keyFrame);
    int updateText(SceneObject *target, KeyFrame *keyFrame);
    int updateTime(SceneObject *target, KeyFrame *keyFrame);
    void updateTrack(std::shared_ptr<TrackPlayback> trackPlayback);
    static std::shared_ptr<KeyFrame> createKeyFrameCb(int type, ANIMATION_COMPLETE_CB, float time);
    static std::shared_ptr<KeyFrame> createKeyFrame(int type, float time);
    static bool cap(float *cur, float target, float dv);
    UpdateData<vec3> updateVector(vec3 original, vec3 desired, vec3 current, KeyFrame *keyFrame);
    UpdateData<string> updateString(string original, string desired, string current, KeyFrame *keyFrame);
    UpdateData<float> updateFloat(float original, float desired, float current, KeyFrame *keyFrame);
    void playTrack(string objectName, string trackName);
    void pauseTrack(string objectName);

 private:
    map<string, KeyFrames> keyFrameStore_;
    map<string, AnimationTracks> trackStore_;
    map<string, std::shared_ptr<TrackPlayback>> activeTracks_;
    std::mutex controllerLock_;
};
