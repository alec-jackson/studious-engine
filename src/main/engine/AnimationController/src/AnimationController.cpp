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

#include <AnimationController.hpp>
#include <UiObject.hpp>

int AnimationController::addKeyframe(SceneObject *target, KeyFrame keyFrame) {
    auto targetName = target->getObjectName();
    // Check if the target object exists in the keyframestore
    auto it = keyFrameStore_.find(targetName);
    auto kfQueueSize = 0;

    // Move the keyFrame onto the heap... #jank
    auto heapFrame = new KeyFrame();
    heapFrame->currentTime = 0.0f;
    heapFrame->pos.desired = keyFrame.pos.desired;
    heapFrame->targetTime = keyFrame.targetTime;
    heapFrame->pos.original = target->getPosition();
    // Use stretch if UI object
    if (target->type() == ObjectType::UI_OBJECT) {
        auto cTarget = (UiObject *)target;
        heapFrame->stretch.original = vec3(cTarget->getWScale(), cTarget->getHScale(), 0.0f);
    }

    // If the target exists in the key store, do some sanity checks...
    if (it != keyFrameStore_.end()) {
        auto keyFrames = it->second;
        // Make sure we do not have duplicate SceneObject names in the store
        assert(keyFrames.target == target);
        // Add the keyFrame to the object's keyframe queue
        keyFrames.kQueue.push(heapFrame);
        kfQueueSize = keyFrames.kQueue.size();
    } else {
        // If the object has no keyframestore, add it
        keyFrameStore_[target->getObjectName()].kQueue.push(heapFrame);
        keyFrameStore_[target->getObjectName()].target = target;
        kfQueueSize = 1;
    }
    return kfQueueSize;
}

void AnimationController::update() {
    printf("AnimationController::update: DeltaTime %f\n", deltaTime);
    vector<string> deferredDelete;
    // Run update methods on each object here
    for (auto &entry : keyFrameStore_) {
        // Grab the front keyFrame for the object
        auto currentKf = entry.second.kQueue.front();
        auto target = entry.second.target;
        auto result = UPDATE_NOT_COMPLETE;
        auto done = POSITION_MET | STRETCH_MET;

        // Update the time passed since keyframe has started
        currentKf->currentTime += deltaTime;
        // Perform updates in keyframe
        result |= updatePosition(target, currentKf);
        result |= updateStretch(target, currentKf);
        // Only remove the keyframe when all updates are done...
        if (result & done) {
            printf("AnimationController::update: Finished keyframe for %s\n", target->getObjectName().c_str());
            // Remove the keyframe from the queue
            entry.second.kQueue.pop();
            free(currentKf);
            deferredDelete.push_back(entry.first);
        }
        // Probably remove this entry from the map after this loop if empty...
    }
    // Erase keys in the deferredDelete list
    for (auto item : deferredDelete) {
        keyFrameStore_.erase(item);
    }
}

int AnimationController::updatePosition(SceneObject *target, KeyFrame *keyFrame) {
    // Caps the position when the position differs than the target
    auto positionCapping = [](float *pos, float target, float dp) {
        auto capped = false;
        auto direction = 0;
        // Use dp to determine direction...
        if (dp > 0.0f) {
            direction = CAP_POS;
        } else if (dp < 0.0f) {
            direction = CAP_NEG;
        }
        switch (direction) {
            case CAP_POS:
                if (*pos > target) {
                    capped = true;
                    *pos = target;
                }
                break;
            case CAP_NEG:
                if (*pos < target) {
                    capped = true;
                    *pos = target;
                }
                break;
            default:
                break;
        }
        return capped;
    };
    auto matchedPos = 0;
    // Translation delta is time per frame * totalTime
    auto targetPos = target->getPosition();
    auto tDx = keyFrame->pos.desired.x - keyFrame->pos.original.x;
    auto tDy = keyFrame->pos.desired.y - keyFrame->pos.original.y;
    auto tDz = keyFrame->pos.desired.z - keyFrame->pos.original.z;

    auto timeScalar = deltaTime / keyFrame->targetTime;
    targetPos.x += (timeScalar * tDx);
    targetPos.y += (timeScalar * tDy);
    targetPos.z += (timeScalar * tDz);

    printf("AnimationController::updatePosition: currentTime %f, [%f][%f][%f]\n",
        keyFrame->currentTime, targetPos.x, targetPos.y, targetPos.z);
    printf("AnimationController::updatePosition: targetTime %f\n", keyFrame->targetTime);

    // Perform position locking when max time is met...
    if (keyFrame->currentTime >= keyFrame->targetTime) {
        targetPos = keyFrame->pos.desired;
        return POSITION_MET;
    }
    // Perform position capping for xyz
    if (positionCapping(&targetPos.x, keyFrame->pos.desired.x, tDx))
        matchedPos++;
    if (positionCapping(&targetPos.y, keyFrame->pos.desired.y, tDy))
        matchedPos++;
    if (positionCapping(&targetPos.z, keyFrame->pos.desired.z, tDz))
        matchedPos++;
    target->setPosition(targetPos);

    return matchedPos == NUM_AXIS ? POSITION_MET : UPDATE_NOT_COMPLETE;
}

int AnimationController::updateStretch(SceneObject *target, KeyFrame *keyFrame) {
    // Update the stretch components for the target (if supported)
    if (target->type() != ObjectType::UI_OBJECT) {
        printf("AnimationController::updateStretch: Stretch on unsupported target %s\n",
            target->getObjectName().c_str());
        return STRETCH_MET;
    }

    UiObject *cTarget = (UiObject *)target;

    // Caps the position when the position differs than the target
    auto stretchCapping = [](float *stretch, float target, float ds) {
        auto capped = false;
        auto direction = 0;
        // Use dp to determine direction...
        if (ds > 0.0f) {
            direction = CAP_POS;
        } else if (ds < 0.0f) {
            direction = CAP_NEG;
        }
        switch (direction) {
            case CAP_POS:
                if (*stretch > target) {
                    capped = true;
                    *stretch = target;
                }
                break;
            case CAP_NEG:
                if (*stretch < target) {
                    capped = true;
                    *stretch = target;
                }
                break;
            default:
                break;
        }
        return capped;
    };
    auto matchedPos = 0;
    // Translation delta is time per frame * totalTime
    auto targetStretch = vec3(cTarget->getWScale(), cTarget->getHScale(), 0.0f);
    auto tDx = keyFrame->stretch.desired.x - keyFrame->stretch.original.x;
    auto tDy = keyFrame->stretch.desired.y - keyFrame->stretch.original.y;
    auto tDz = keyFrame->stretch.desired.z - keyFrame->stretch.original.z;

    auto timeScalar = deltaTime / keyFrame->targetTime;
    targetStretch.x += (timeScalar * tDx);
    targetStretch.y += (timeScalar * tDy);
    targetStretch.z += (timeScalar * tDz);

    printf("AnimationController::updateStretch: currentTime %f, [%f][%f][%f]\n",
        keyFrame->currentTime, targetStretch.x, targetStretch.y, targetStretch.z);
    printf("AnimationController::updateStretch: targetTime %f\n", keyFrame->targetTime);

    // Perform position locking when max time is met...
    if (keyFrame->currentTime >= keyFrame->targetTime) {
        targetStretch = keyFrame->stretch.desired;
        return POSITION_MET;
    }
    // Perform position capping for xyz
    if (stretchCapping(&targetStretch.x, keyFrame->stretch.desired.x, tDx))
        matchedPos++;
    if (stretchCapping(&targetStretch.y, keyFrame->stretch.desired.y, tDy))
        matchedPos++;
    if (stretchCapping(&targetStretch.z, keyFrame->stretch.desired.z, tDz))
        matchedPos++;
    cTarget->setWScale(targetStretch.x);
    cTarget->setHScale(targetStretch.y);

    return matchedPos == NUM_AXIS ? STRETCH_MET : UPDATE_NOT_COMPLETE;

}
