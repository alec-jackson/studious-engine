/**
 * @file ImageExt.hpp
 * @author Christian Galvez
 * @brief Header file for ImageExt class extension for SceneObjects. This allows SceneObjects to
 * implement Image based transformations in the AnimationController.
 * @date 09-05-2025
 * @copyright studious-engine 2025
 */
#pragma once
#include <glm/glm.hpp>

using glm::vec4;

class ImageExt {
 public:
    ImageExt();
    inline void setTint(vec4 tint) { tint_ = tint; }
    inline vec4 getTint() const { return tint_; }
 protected:
    vec4 tint_;
};
