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

#include <map>
#include <queue>
#include <cassert>
#include <SceneObject.hpp>

#define UPDATE_NOT_COMPLETE 0
#define POSITION_MET 1
#define STRETCH_MET 2
#define CAP_POS 1
#define CAP_NEG 2
#define NUM_AXIS 3

extern double deltaTime;

template <typename T>
class AnimationData {
public:
    T desired;
    T original;
};

typedef struct KeyFrame {
    AnimationData<vec3> pos;
    AnimationData<vec3> stretch;
    float targetTime;
    float currentTime = 0.0f;
} KeyFrame;

typedef struct KeyFrames {
    std::queue<KeyFrame *> kQueue;
    SceneObject *target;
} KeyFrames;

class AnimationController {
public:
    int addKeyframe(SceneObject *target, KeyFrame *keyFrame);
    void update();
    int updatePosition(SceneObject *target, KeyFrame *keyFrame);
    int updateStretch(SceneObject *target, KeyFrame *keyFrame);
    static KeyFrame *createKeyFrame(vec3 pos, vec3 stretch, float time);
private:
    map<string, KeyFrames> keyFrameStore_;
};
