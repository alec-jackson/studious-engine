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
#include <AnimationController.hpp>
#include <UiObject.hpp>
#include <TextObject.hpp>

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

void AnimationController::addTrack(SpriteObject *target, string trackName, vector<int> trackData, int fps) {
    std::unique_lock<std::mutex> scopeLock(controllerLock_);
    // Make sure the target is not null
    assert(target != nullptr);
    /* If trackData is empty, use all available frames */
    if (trackData.empty()) {
        for (int i = 0; i < target->getBankSize(); ++i) {
            trackData.push_back(i);
        }
    }
    auto track = std::make_shared<AnimationTrack>(
        trackData,
        trackName,
        fps);

    auto it = trackStore_.find(target->getObjectName());

    if (it != trackStore_.end()) {
        /* If the item exists in the map, add the track to the tracKStore entry */
        it->second.tracks[trackName] = track;
    } else {
        /* If the item doesn't exist, create the entry */
        trackStore_[target->getObjectName()].target = target;
        trackStore_[target->getObjectName()].tracks[trackName] = track;
    }
}

void AnimationController::playTrack(string objectName, string trackName) {
    std::unique_lock<std::mutex> scopeLock(controllerLock_);
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
        /* If the track is already running, we'll just restart it and fall through */
    }
    /* Check if any tracks are playing, and pause them */
    auto storeit = trackStore_.find(objectName);
    /* Object does not exist in track store */
    assert(storeit != trackStore_.end());
    auto objectPtr = storeit->second.target;
    auto it = trackStore_[objectName].tracks.find(trackName);
    /* Track does not exist for the object */
    assert(it != trackStore_[objectName].tracks.end());
    /* @todo Probably do something useful here on RELEASE code for failures */
    // Create a TrackPlayback and add it to the playing queue
    float secondsPerFrame = 1.0 / it->second.get()->targetFps;
    printf("AnimationController::playTrack: Starting track %s\n",
        trackName.c_str());
    auto tp = std::make_shared<TrackPlayback>(
        it->second,
        secondsPerFrame,
        secondsPerFrame * it->second.get()->trackData.size(),
        0,
        objectPtr);
    activeTracks_[objectName] = tp;
}

/* Might want to do something fancy for resume */
void AnimationController::pauseTrack(string objectName) {
    std::unique_lock<std::mutex> scopeLock(controllerLock_);
    auto it = activeTracks_.find(objectName);
    if (it != activeTracks_.end()) {
        it->second.get()->state = TrackState::PAUSED;
    } else {
        fprintf(stderr, "AnimationController::pauseTrack: %s has no active animations!",
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
        it->second.kQueue.push(keyFrame.get());
        kfQueueSize = it->second.kQueue.size();
    } else {
        // If the object has no keyframestore, add it
        keyFrameStore_[target->getObjectName()].kQueue.push(keyFrame.get());
        keyFrameStore_[target->getObjectName()].target = target;
        kfQueueSize = 1;
    }
    return kfQueueSize;
}

void AnimationController::update() {
    // Lock the controller
    std::unique_lock<std::mutex> scopeLock(controllerLock_);
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
        }

        auto result = UPDATE_NOT_COMPLETE;
        auto done = POSITION_MET | STRETCH_MET | TEXT_MET | TIME_MET | ROTATION_MET | SCALE_MET;

        // Update the time passed since keyframe has started
        currentKf->currentTime += deltaTime;
        // Perform updates in keyframe
        result |= updatePosition(target, currentKf);
        result |= updateStretch(target, currentKf);
        result |= updateText(target, currentKf);
        result |= updateTime(target, currentKf);
        result |= updateRotation(target, currentKf);
        result |= updateScale(target, currentKf);
        // Only remove the keyframe when all updates are done...
        if (result == done) {
            printf("AnimationController::update: Finished keyframe for %s\n", target->getObjectName().c_str());
            // Remove the keyframe from the queue
            entry.second.kQueue.pop();
            // Call the callback associated with the keyframe
            if (currentKf->hasCb) currentKf->callback();
            if (entry.second.kQueue.empty())
                deferredDelete.push_back(entry.first);
        }
        // Probably remove this entry from the map after this loop if empty...
    }
    // Erase keys in the deferredDelete list
    for (auto item : deferredDelete) {
        keyFrameStore_.erase(item);
    }
    /* Update track based animations */
    for (auto &entry : activeTracks_) {
        /* Only update the track if it's running */
        auto state = entry.second.get()->state;
        if (state == TrackState::RUNNING)
            /* Update the active track */
            updateTrack(entry.second);
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
        target->getPosition(),
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
        target->getRotation(),
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
        target->getScale(),
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
        cTarget->getStretch(),
        keyFrame);
    cTarget->setWStretch(updated.updatedValue_.x);
    cTarget->setHStretch(updated.updatedValue_.y);

    return (updated.updateComplete_) ? STRETCH_MET : UPDATE_NOT_COMPLETE;
}

bool AnimationController::cap(float *cur, float target, float dv) {
        auto capped = false;
        auto direction = 0;
        // Use dp to determine direction...
        if (dv > 0.0f) {
            direction = CAP_POS;
        } else if (dv < 0.0f) {
            direction = CAP_NEG;
        }
        switch (direction) {
            case CAP_POS:
                if (*cur > target) {
                    capped = true;
                    *cur = target;
                }
                break;
            case CAP_NEG:
                if (*cur < target) {
                    capped = true;
                    *cur = target;
                }
                break;
            default:
                break;
        }
        return  capped || *cur == target;
}

UpdateData<vec3> AnimationController::updateVector(vec3 original, vec3 desired, vec3 current, KeyFrame *keyFrame) {
    // Caps the position when the position differs than the target
    auto matchedPos = 0;
    auto timeMet = 0;
    auto updateResult = false;
    // Translation delta is time per frame * totalTime
    auto tD = desired - original;

    auto timeScalar = static_cast<float>(deltaTime) / keyFrame->targetTime;
    current += (tD * timeScalar);

    // Perform position locking when max time is met...
    if (keyFrame->currentTime >= keyFrame->targetTime) {
        current = desired;
        timeMet = 1;
    }
    // Perform position capping for xyz
    for (int i = 0; i < 3; ++i) {
        if (cap(&current[i], desired[i], tD[i]))
        matchedPos++;
    }
    updateResult = (matchedPos == NUM_AXIS && timeMet);
    return UpdateData<vec3>(current, updateResult);
}

UpdateData<float> AnimationController::updateFloat(float original, float desired, float current, KeyFrame *keyFrame) {
    // Caps the position when the position differs than the target
    auto matched = false;
    auto timeMet = 0;
    auto updateResult = false;
    // Translation delta is time per frame * totalTime
    auto tD = desired - original;

    auto timeScalar = static_cast<float>(deltaTime) / keyFrame->targetTime;
    current += (tD * timeScalar);

    // Perform position locking when max time is met...
    if (keyFrame->currentTime >= keyFrame->targetTime) {
        current = desired;
        timeMet = 1;
    }
    // Perform float capping
    if (cap(&current, desired, tD)) matched = true;
    updateResult = (matched && timeMet);
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

void AnimationController::updateTrack(std::shared_ptr<TrackPlayback> trackPlayback) {
    auto tp = trackPlayback.get();
    auto target = trackPlayback.get()->target;
    auto track = tp->track.get()->trackData;
    /* Update timings and current frame */
    tp->currentTime += deltaTime;
    /* Wrap time around sequence time */
    tp->currentTime = std::fmod(tp->currentTime, tp->sequenceTime);
    /* Determine current frame based on current time */
    int trackIdx = (tp->currentTime / tp->sequenceTime) * tp->track.get()->trackData.size();
    tp->currentTrackIdx = trackIdx;
    /* Grab the real frame number with the track idx */
    auto frameNumber = tp->track.get()->trackData.at(trackIdx);
    /* Set the sprite object's frame number to the frame number calculated */
    target->setCurrentFrame(frameNumber);
}
