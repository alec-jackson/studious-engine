#include <SceneObject.hpp>
#include <set>

using std::set;

#define TEST_OBJECT_NAME "testObject"
#define PARENT_OBJECT_NAME "parentObject"

class TestObject : public SceneObject {
 public:
    TestObject();
    explicit TestObject(string name);
    void render() override;
    void update() override;
    inline SceneObject *getParent() { return parent_; }
    inline const set<SceneObject *> &getChildren() { return children_; }
    inline const mat4 &getTranslationMatrix() { return translateMatrix_; }
    inline const mat4 &getRotationMatrix() { return rotateMatrix_; }
    inline const mat4 &getScaleMatrix() { return scaleMatrix_; }
};
