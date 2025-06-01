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
#include "SceneObject.hpp"
#include <map>
#include <queue>
#include <cassert>
#include <string>
#include <functional>
#include <mutex> // NOLINT
#include <memory>
#include <vector>
#include <TrackExt.hpp>
#include <UiObject.hpp>
#include <TextObject.hpp>

// Update return values
#define UPDATE_NOT_COMPLETE 0
#define POSITION_MET 1
#define STRETCH_MET 2
#define TEXT_MET 4
#define TIME_MET 8
#define ROTATION_MET 16
#define SCALE_MET 32
#define COLOR_MET 64

// Update Types
#define UPDATE_NONE 0
#define UPDATE_POS 1
#define UPDATE_STRETCH 2
#define UPDATE_TEXT 4
#define UPDATE_TIME 8
#define UPDATE_ROTATION 16
#define UPDATE_SCALE 32
#define UPDATE_COLOR 64

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

/**
 * @brief Configuration for an animation track.
 */
struct TrackConfiguration {
    vector<int> trackData;
    string trackName;
    int targetFps;
    bool loop;
    inline TrackConfiguration(vector<int> tD, string tN, int tF, bool l) :
        trackData { tD }, trackName { tN }, targetFps { tF }, loop { l } {};
};

/**
 * @brief Contains a set of tracks for a target object.
 */
struct TrackStoreEntry {
    TrackExt *target;
    std::shared_ptr<TrackConfiguration> track;
};

/**
 * @brief Entry object for the active tracks list. Contains playback information
 * for an active animation track.
 */
struct ActiveTrackEntry {
    TrackState state = TrackState::RUNNING;
    std::shared_ptr<TrackConfiguration> track;
    float secondsPerFrame;
    float sequenceTime;
    float currentTime = 0.0f;
    int currentTrackIdx;
    TrackExt *target;
    inline ActiveTrackEntry(std::shared_ptr<TrackConfiguration> tr, float sPF, float sT, int cTI, TrackExt *ta) :
        track { tr }, secondsPerFrame { sPF }, sequenceTime { sT }, currentTrackIdx { cTI }, target { ta } {};
};

struct KeyFrame {
    AnimationData<vec3> pos;
    AnimationData<vec3> stretch;
    AnimationData<string> text;
    AnimationData<vec3> rotation;
    AnimationData<float> scale;
    AnimationData<vec4> color;
    float targetTime;
    float currentTime = 0.0f;
    int type;
    ANIMATION_COMPLETE_CB;
    bool hasCb;
    bool isNew = true;
};

struct KeyFrames {
    std::queue<std::shared_ptr<KeyFrame>> kQueue;
    SceneObject *target;
};

class AnimationController {
 public:
    int addKeyFrame(SceneObject *target, std::shared_ptr<KeyFrame> keyFrame);
    void addTrack(TrackExt *target, string trackName, vector<int> trackData, int fps, bool loop);
    void update();
    int updatePosition(SceneObject *target, KeyFrame *keyFrame);
    int updateRotation(SceneObject *target, KeyFrame *keyFrame);
    int updateScale(SceneObject *target, KeyFrame *keyFrame);
    int updateStretch(SceneObject *target, KeyFrame *keyFrame);
    int updateText(SceneObject *target, KeyFrame *keyFrame);
    int updateTime(SceneObject *target, KeyFrame *keyFrame);
    int updateColor(SceneObject *target, KeyFrame *keyFrame);
    bool updateTrack(std::shared_ptr<ActiveTrackEntry> trackPlayback);
    static std::shared_ptr<KeyFrame> createKeyFrameCb(int type, ANIMATION_COMPLETE_CB, float time);
    static std::shared_ptr<KeyFrame> createKeyFrame(int type, float time);
    static bool cap(float *cur, float target, float dv);
    UpdateData<vec3> updateVector(vec3 original, vec3 desired, vec3 current, KeyFrame *keyFrame);
    UpdateData<vec4> updateVector4(vec4 original, vec4 desired, vec4 current, KeyFrame *keyFrame);
    UpdateData<string> updateString(string original, string desired, string current, KeyFrame *keyFrame);
    UpdateData<float> updateFloat(float original, float desired, float current, KeyFrame *keyFrame);
    void playTrack(string trackName);
    void pauseTrack(string trackName);
    /**
     * @brief Removes a scene object from all track and key frame stores in the AnimationController.
     * @param objectName Name of the object to remove from the AnimationController.
     */
    void removeSceneObject(string objectName);
    // Getters for testing
    inline const std::map<string, KeyFrames> &getKeyFrameStore() { return keyFrameStore_; }
    inline const std::map<string, TrackStoreEntry> &getTrackStore() { return trackStore_; }
    inline const std::map<string, std::shared_ptr<ActiveTrackEntry>> &getActiveTracks() { return activeTracks_; }

 private:
    map<string, KeyFrames> keyFrameStore_;
    /* Map of trackName to TrackConfig */
    map<string, TrackStoreEntry> trackStore_;
    /* Map of object name to active track */
    map<string, std::shared_ptr<ActiveTrackEntry>> activeTracks_;
    std::mutex controllerLock_;
};
