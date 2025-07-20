/**
 * @file AnimationController.cpp
 * @author Christian Galvez
 * @brief Implementation for Animation Controller
 * @version 0.1
 * @date 2024-10-20
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <vector>
#include <string>
#include <cstdio>
#include <memory>
#include <cmath>
#include <algorithm>
#include <AnimationController.hpp>

std::shared_ptr<KeyFrame> AnimationController::createKeyFrameCb(int type, ANIMATION_COMPLETE_CB, float time) {
    auto keyframe = createKeyFrame(type, time);
    keyframe.get()->callback = callback;
    keyframe.get()->hasCb = true;
    return keyframe;
}

std::shared_ptr<KeyFrame> AnimationController::createKeyFrame(int type, float time) {
    auto keyframe = std::make_shared<KeyFrame>();
    keyframe.get()->targetTime = time;
    keyframe.get()->currentTime = 0.0f;
    keyframe.get()->type = type;
    keyframe.get()->hasCb = false;
    return keyframe;
}

/**
 * @brief Creates and adds a track configuration to the internal track store. Adding a track to the
 * track store will not automatically play it. @see AnimationController::playTrack.
 * @param target The TrackExt to apply the animation track to.
 * @param trackName Friendly name of the animation track.
 * @param trackData The actual track data. Each number in the list corresponds to a frame to set in the TrackExt's
 * sprite grid. For example, the trackData { 3, 4, 5 } means that the animation track will first display frame 3, then
 * 4 and then 5. The speed at which frames are sequentially switched is determined by the supplied fps rate. An empty
 * vector for trackData is actually legal, and will default to a set of increasing numbers starting from 0 to the
 * number of available frames in the TrackExt. For a TrackExt with 4 frames, the default trackData would look
 * like { 0, 1, 2, 3 }.
 * @param fps The framerate the animation should play back.
 * @param loop Will control whether the animation loops infinitely or ends on last frame.
 * @note There is no trackData bounds checking at this level. Bounds checking occurs at the TrackExt level.
 *
 * There are two internal AnimationController maps that are used to store and play track data.
 * trackStore_ -> Internal map of tracks.
 * activeTracks_ -> This is a map of object names to ActiveTrackEntry. An ActiveTrackEntry is just playback information
 * for a single track from the trackStore_ map. There can only ever be ONE animation track per object EVER!!! This
 * means if you call AnimationController::playTrack on an object that already has an active track, the previous track
 * will be stopped and replaced with the new track. Memory is managed through smart pointers, so everything should
 * clean up on its own.
 */
void AnimationController::addTrack(TrackExt *target, string trackName, vector<int> trackData, int fps, bool loop) {
    std::unique_lock<std::mutex> scopeLock(controllerLock_);
    if (target == nullptr) {
        fprintf(stderr, "AnimationController::addTrack: target cannot be null.\n");
        return;
    }
    /* If trackData is empty, use all available frames */
    if (trackData.empty()) {
        for (uint i = 0; i < target->getBankSize(); ++i) {
            trackData.push_back(i);
        }
    }

    auto track = std::make_shared<TrackConfiguration>(
        trackData,
        trackName,
        fps,
        loop);

    /* Set the track in the track store */
    trackStore_[trackName].target = target;
    trackStore_[trackName].track = track;
}

/**
 * @brief Plays a track for the given object. The supplied trackName MUST be already defined in the track store to
 * be played. Otherwise the animation will not start, and an error will be printed.
 * @param trackName The name of the track to play.
 * @note Will return early and not play the animation if the track does not exist. If the active track for the object
 * the same as the queried one through trackName, then there are some special cases. If the active track was paused,
 * then it will be resumed WHERE IT LEFT OFF. If the active track is currently running, then it will be replaced and
 * started from the beginning. This is a slight nuance, but is important to know about for proper usage.
 */
void AnimationController::playTrack(string trackName) {
    std::unique_lock<std::mutex> scopeLock(controllerLock_);
    /* Check if the requested track exists */
    auto tsit = trackStore_.find(trackName);
    if (tsit == trackStore_.end()) {
        fprintf(stderr, "AnimationController::playTrack: %s does not exist in the track store.\n",
            trackName.c_str());
        return;
    }
    auto trackPtr = tsit->second.target;
    auto objectPtr = trackPtr->getObj();
    auto objectName = objectPtr->getObjectName();
    /* Check if the animation is still in the active list */
    auto ait = activeTracks_.find(objectName);
    if (ait != activeTracks_.end()) {
        auto match = ait->second.get()->track.get()->trackName.compare(trackName);
        auto &state = ait->second.get()->state;
        /* If the active track is the same track, let's resume it if paused... */
        if (!match && state == TrackState::PAUSED) {
            /* Resume the animation from where it left off if so */
            ait->second.get()->state = TrackState::RUNNING;
            printf("AnimationController::playTrack: Resuming previously active track %s\n",
                trackName.c_str());
            return;
        }
        /* If the track is already running, we'll just fall through and restart it */
    }
    // Create a ActiveTrackEntry and add it to the playing queue
    float secondsPerFrame = 1.0 / tsit->second.track.get()->targetFps;
    printf("AnimationController::playTrack: Starting track %s\n",
        trackName.c_str());
    auto tp = std::make_shared<ActiveTrackEntry>(
        tsit->second.track,
        secondsPerFrame,
        secondsPerFrame * tsit->second.track.get()->trackData.size(),
        0,
        trackPtr);
    activeTracks_[objectName] = tp;
}

/**
 * @brief Pauses the animation track's playback. Will do nothing if the track does not exist or is not running.
 * @param trackName The name of the track to pause.
 */
void AnimationController::pauseTrack(string trackName) {
    std::unique_lock<std::mutex> scopeLock(controllerLock_);
    /* Check the object store for the track */
    auto sit = trackStore_.find(trackName);
    if (sit == trackStore_.end()) {
        fprintf(stderr,
            "AnimationController::pauseTrack: %s does not exist in the track store. Cannot pause animation.\n",
            trackName.c_str());
        return;
    }
    auto objectName = sit->second.target->getObj()->getObjectName();
    auto it = activeTracks_.find(objectName);
    if (it != activeTracks_.end()) {
        it->second.get()->state = TrackState::PAUSED;
    } else {
        fprintf(stderr, "AnimationController::pauseTrack: %s has no active animations.",
            objectName.c_str());
    }
}

int AnimationController::addKeyFrame(SceneObject *target, std::shared_ptr<KeyFrame> keyFrame) {
    std::unique_lock<std::mutex> scopeLock(controllerLock_);
    auto targetName = target->getObjectName();
    // Check if the target object exists in the keyframestore
    auto it = keyFrameStore_.find(targetName);
    auto kfQueueSize = 0;

    // If the target exists in the key store, do some sanity checks...
    if (it != keyFrameStore_.end()) {
        // Make sure we do not have duplicate SceneObject names in the store
        assert(it->second.target == target);
        // Add the keyFrame to the object's keyframe queue
        it->second.kQueue.push(keyFrame);
        kfQueueSize = it->second.kQueue.size();
    } else {
        // If the object has no keyframestore, add it
        keyFrameStore_[target->getObjectName()].kQueue.push(keyFrame);
        keyFrameStore_[target->getObjectName()].target = target;
        kfQueueSize = 1;
    }
    return kfQueueSize;
}

void AnimationController::update() {
    // Lock the controller
    std::unique_lock<std::mutex> scopeLock(controllerLock_);
    vector<std::function<void(void)>> callbacks;
    vector<string> deferredDelete;
    // Run update methods on each object here
    for (auto &entry : keyFrameStore_) {
        // Grab the front keyFrame for the object
        auto currentKf = entry.second.kQueue.front();
        auto target = entry.second.target;
        // If this is a brand new keyframe, set original values...
        if (currentKf->isNew) {
            currentKf->isNew = false;
            currentKf->pos.original = target->getPosition();
            currentKf->rotation.original = target->getRotation();
            currentKf->scale.original = target->getScale();
            // Use stretch if UI object
            if (currentKf->type & UPDATE_STRETCH) {
                assert(target->type() == ObjectType::UI_OBJECT);
                auto cTarget = static_cast<UiObject *>(target);
                currentKf->stretch.original = cTarget->getStretch();
            }
            if (currentKf->type & UPDATE_TEXT) {
                assert(target->type() == ObjectType::TEXT_OBJECT);
                auto cTarget = static_cast<TextObject *>(target);
                currentKf->text.original = cTarget->getMessage();
            }
            if (currentKf->type & UPDATE_COLOR) {
                assert(target->type() == ObjectType::TEXT_OBJECT);
                auto cTarget = static_cast<TextObject *>(target);
                currentKf->color.original = cTarget->getColor();
            }
        }

        auto result = UPDATE_NOT_COMPLETE;
        auto done = POSITION_MET | STRETCH_MET | TEXT_MET | TIME_MET | ROTATION_MET | SCALE_MET | COLOR_MET;
        auto &currentTime = currentKf.get()->currentTime;
        auto &targetTime = currentKf.get()->targetTime;
        // Update the time passed since keyframe has started
        currentTime = std::min<float>(currentTime + deltaTime, targetTime);
        // Perform updates in keyframe
        result |= updatePosition(target, currentKf.get());
        result |= updateStretch(target, currentKf.get());
        result |= updateText(target, currentKf.get());
        result |= updateTime(target, currentKf.get());
        result |= updateRotation(target, currentKf.get());
        result |= updateScale(target, currentKf.get());
        result |= updateColor(target, currentKf.get());
        // Only remove the keyframe when all updates are done...
        if (result == done) {
            printf("AnimationController::update: Finished keyframe for %s\n", target->getObjectName().c_str());
            // Remove the keyframe from the queue
            entry.second.kQueue.pop();
            // Call the callback associated with the keyframe
            if (currentKf->hasCb) callbacks.push_back(currentKf->callback);
            if (entry.second.kQueue.empty())
                deferredDelete.push_back(entry.first);
        }
        // Probably remove this entry from the map after this loop if empty...
    }
    // Erase keys in the deferredDelete list
    for (auto item : deferredDelete) {
        keyFrameStore_.erase(item);
    }
    deferredDelete.clear();
    /* Update track based animations */
    for (auto &entry : activeTracks_) {
        /* Only update the track if it's running */
        auto state = entry.second.get()->state;
        if (state == TrackState::RUNNING) {
            /* Update the active track */
            if (updateTrack(entry.second)) {
                deferredDelete.push_back(entry.first);
            }
        }
    }
    // Erase keys in the deferredDelete list
    for (auto item : deferredDelete) {
        activeTracks_.erase(item);
    }
    /* Run callbacks after update() with lock released */
    scopeLock.unlock();
    for (auto cb : callbacks) {
        cb();
    }
}

int AnimationController::updatePosition(SceneObject *target, KeyFrame *keyFrame) {
    // Only update if the keyframe type has POSITION
    if (!(keyFrame->type & UPDATE_POS)) {
        return POSITION_MET;
    }
    auto result = updateVector(
        keyFrame->pos.original,
        keyFrame->pos.desired,
        keyFrame);

    target->setPosition(result.updatedValue_);

    return (result.updateComplete_) ? POSITION_MET : UPDATE_NOT_COMPLETE;
}

int AnimationController::updateRotation(SceneObject *target, KeyFrame *keyFrame) {
    // Only update if the keyframe type has POSITION
    if (!(keyFrame->type & UPDATE_ROTATION)) {
        return ROTATION_MET;
    }
    auto result = updateVector(
        keyFrame->rotation.original,
        keyFrame->rotation.desired,
        keyFrame);

    target->setRotation(result.updatedValue_);

    return (result.updateComplete_) ? ROTATION_MET : UPDATE_NOT_COMPLETE;
}

int AnimationController::updateScale(SceneObject *target, KeyFrame *keyFrame) {
    // Only update if the keyframe type has POSITION
    if (!(keyFrame->type & UPDATE_SCALE)) {
        return SCALE_MET;
    }
    auto result = updateFloat(
        keyFrame->scale.original,
        keyFrame->scale.desired,
        keyFrame);

    target->setScale(result.updatedValue_);

    return (result.updateComplete_) ? SCALE_MET : UPDATE_NOT_COMPLETE;
}

int AnimationController::updateStretch(SceneObject *target, KeyFrame *keyFrame) {
    // Only update if the keyframe type is stretch
    if (!(keyFrame->type & UPDATE_STRETCH)) {
        return STRETCH_MET;
    }
    // Update the stretch components for the target (if supported)
    if (target->type() != ObjectType::UI_OBJECT) {
        fprintf(stderr, "AnimationController::updateStretch: Stretch on unsupported target %s\n",
            target->getObjectName().c_str());
        // Unsupported update type, assert
        assert(false);
    }

    UiObject *cTarget = reinterpret_cast<UiObject *>(target);
    auto updated = updateVector(keyFrame->stretch.original,
        keyFrame->stretch.desired,
        keyFrame);
    cTarget->setWStretch(updated.updatedValue_.x);
    cTarget->setHStretch(updated.updatedValue_.y);

    return (updated.updateComplete_) ? STRETCH_MET : UPDATE_NOT_COMPLETE;
}

int AnimationController::updateColor(SceneObject *target, KeyFrame *keyFrame) {
    // Only update if the keyframe type has COLOR
    if (!(keyFrame->type & UPDATE_COLOR)) {
        return COLOR_MET;
    }
    // Check if the target is a text object
    if (target->type() != ObjectType::TEXT_OBJECT) {
        // This is horrible, log the error and assert
        fprintf(stderr,
            "AnimationController::updateColor: Attempting to update non-text object %s!\n",
            target->getObjectName().c_str());
        assert(false);
    }
    auto cTarget = reinterpret_cast<TextObject *>(target);
    auto result = updateVector(
        keyFrame->color.original,
        keyFrame->color.desired,
        keyFrame);

    cTarget->setColor(result.updatedValue_);

    return (result.updateComplete_) ? COLOR_MET : UPDATE_NOT_COMPLETE;
}

float AnimationController::linearFloatTransform(float original, float desired, KeyFrame *keyFrame) {
    float delta = desired - original;  // Change in values
    float timeScale = 1.0f;
    if (keyFrame->targetTime != 0.0f)  // Safety
        timeScale = keyFrame->currentTime / keyFrame->targetTime;  // % of transformation
    return original + (delta * timeScale);
}

template <typename T>
UpdateData<T> AnimationController::updateVector(T original, T desired, KeyFrame *keyFrame) {
    auto updateResult = keyFrame->currentTime >= keyFrame->targetTime;
    // Perform position capping for dimensions
    uint containerSize = sizeof(T) / sizeof(float);
    T current;
    for (uint i = 0; i < containerSize; ++i) {
        current[i] = linearFloatTransform(original[i], desired[i], keyFrame);
    }
    return UpdateData<T>(current, updateResult);
}

UpdateData<float> AnimationController::updateFloat(float original, float desired, KeyFrame *keyFrame) {
    // Caps the position when the position differs than the target
    auto updateResult = keyFrame->currentTime >= keyFrame->targetTime;

    auto current = linearFloatTransform(original, desired, keyFrame);
    return UpdateData<float>(current, updateResult);
}

UpdateData<string> AnimationController::updateString(string original, string desired,
    string current, KeyFrame *keyFrame) {
    // Do the delta math (per line??)
    auto timeMet = false;
    // Math will be different than vectors, transformations will NOT be anything crazy
    // Will only support string shrinking and growing...
    auto deltaLength = desired.length() - original.length();
    auto timePercentage = keyFrame->currentTime / keyFrame->targetTime;
    auto dLength = 1.0f / deltaLength;
    auto characterLength = static_cast<int>(timePercentage / dLength) + original.length();

    // Use the timePercentage to add characters to the current string
    if (current.compare(desired) != 0)
        current = desired.substr(0, characterLength);

    if (keyFrame->currentTime >= keyFrame->targetTime) {
        current = desired;
        timeMet = true;
    }

    return UpdateData<string>(current, timeMet);
}

int AnimationController::updateText(SceneObject *target, KeyFrame *keyFrame) {
    // Check if the keyframe type has text
    if (!(keyFrame->type & UPDATE_TEXT)) {
        return TEXT_MET;
    }
    // Check if the target is a text object
    if (target->type() != ObjectType::TEXT_OBJECT) {
        // This is horrible, log the error and assert
        fprintf(stderr,
            "AnimationController::updateText: Attempting to update non-text object %s!\n",
            target->getObjectName().c_str());
        assert(false);
    }
    auto cTarget = reinterpret_cast<TextObject *>(target);
    auto result = updateString(
        keyFrame->text.original,
        keyFrame->text.desired,
        cTarget->getMessage(),
        keyFrame);

    cTarget->setMessage(result.updatedValue_);

    return result.updateComplete_ ? TEXT_MET : UPDATE_NOT_COMPLETE;
}

int AnimationController::updateTime(SceneObject *target, KeyFrame *keyFrame) {
    // Literally just check if we've reached the time quota
    auto result = UPDATE_NOT_COMPLETE;
    if (keyFrame->currentTime >= keyFrame->targetTime) {
        result = UPDATE_TIME;
    }
    return result;
}

/**
 * @brief Updates the currently rendered frame of the target TrackExt based on framerate of animation track,
 * deltaTime, and data from the track.
 * @param trackPlayback The active track to update.
 * @return true if the track is complete, false if it's still ongoing.
 */
bool AnimationController::updateTrack(std::shared_ptr<ActiveTrackEntry> trackPlayback) {
    auto tp = trackPlayback.get();
    auto target = trackPlayback.get()->target;
    auto track = tp->track.get()->trackData;
    /* Update timings and current frame */
    tp->currentTime += deltaTime;
    /* Break the update early if loop is enabled */
    if (!tp->track.get()->loop && tp->currentTime >= tp->sequenceTime) {
        target->setCurrentFrame(tp->track.get()->trackData.at(tp->track.get()->trackData.size() - 1));
        return true;
    }
    /* Wrap time around sequence time */
    tp->currentTime = std::fmod(tp->currentTime, tp->sequenceTime);
    /* Determine current frame based on current time */
    int trackIdx = (tp->currentTime / tp->sequenceTime) * tp->track.get()->trackData.size();
    tp->currentTrackIdx = trackIdx;
    /* Grab the real frame number with the track idx */
    auto frameNumber = tp->track.get()->trackData.at(trackIdx);
    /* Set the sprite object's frame number to the frame number calculated */
    target->setCurrentFrame(frameNumber);
    return false;
}

void AnimationController::removeSceneObject(string objectName) {
    std::unique_lock<std::mutex> scopeLock(controllerLock_);
    // Delete active tracks
    auto atit = activeTracks_.find(objectName);
    if (atit != activeTracks_.end())
        activeTracks_.erase(atit);

    // Delete track stores
    vector<string> toDelete;
    for (auto entry : trackStore_) {
        if (entry.second.target->getObj()->getObjectName().compare(objectName) == 0) {
            toDelete.push_back(entry.first);
        }
    }
    for (auto entry : toDelete) {
        auto tsit = trackStore_.find(entry);
        trackStore_.erase(tsit);
    }
    // delete keyframes
    auto kfit = keyFrameStore_.find(objectName);
    if (kfit != keyFrameStore_.end())
        keyFrameStore_.erase(kfit);
}
