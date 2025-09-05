#pragma once
#include <glm/glm.hpp>

using glm::vec4;

class ImageExt {
 public:
    inline void setTint(vec4 tint) { tint_ = tint; }
    inline vec4 getTint() const { return tint_; }
 protected:
    vec4 tint_;
};
