/**
 * @file SceneObjectTests.cpp
 * @author Christian Galvez
 * @brief Test suite for SceneObject functions.
 * @version 0.1
 * @date 2025-07-13
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <SceneObjectTests.hpp>
#include <gtest/gtest.h>
#include <vector>
#include <memory>
#include <iostream>
#include <string>
#include <set>

using std::endl;
using std::cout;
using std::set;

const char *TEST_OBJECT_NAME = "testObject";
const char *PARENT_OBJECT_NAME = "parentObject";

/**
 * @brief Launches google test suite defined in file
 *
 * @param argc
 * @param argv
 * @return int
 */
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    cout << "Running GTESTS" << endl;
    auto result = RUN_ALL_TESTS();
    if (!result) {
        cout << "All tests passed" << endl;
    } else {
        cout << "Some test failures detected!" << endl;
    }

    return result;
}

class FakeSceneObject : public SceneObject {
 public:
    FakeSceneObject();
    explicit FakeSceneObject(string name);
    void render() override;
    void update() override;
    SceneObject *getParent() { return parent_; }
    const set<SceneObject *> &getChildren() { return children_; }
    const mat4 &getTranslationMatrix() { return translateMatrix_; }
    const mat4 &getRotationMatrix() { return rotateMatrix_; }
    const mat4 &getScaleMatrix() { return scaleMatrix_; }
};

FakeSceneObject::FakeSceneObject() : SceneObject(ObjectType::UNDEFINED, TEST_OBJECT_NAME, nullptr) {
}

FakeSceneObject::FakeSceneObject(string name) : SceneObject(ObjectType::UNDEFINED, name, nullptr) {
}

void FakeSceneObject::render() {
}

void FakeSceneObject::update() {
}

class GivenASceneObject: public ::testing::Test {
 protected:
    FakeSceneObject object_;
};

/**
 * @brief Ensures that newly constructed SceneObjects do not have any parents or children.
 */
TEST_F(GivenASceneObject, WhenConstructed_ThenNoParentOrChildrenPresent) {
    /* Preparation / Action / Validation */
    ASSERT_TRUE(object_.getParent() == nullptr);
    ASSERT_TRUE(object_.getChildren().empty());
}

/**
 * @brief Ensures an object with no parent will update its respective matrices when
 * updateModelMatrices is called.
 */
TEST_F(GivenASceneObject, WhenUpdateModelMatricesNoParent_ThenUpdatedAsExpected) {
    /* Preparation */
    vec3 examplePosition(1.0f, 2.0f, 3.0f);
    vec3 exampleRotation(2.0f, 3.0f, 4.0f);
    float exampleScale = 2.2f;

    mat4 expectedTm = glm::translate(examplePosition);
    mat4 expectedRm = glm::rotate(glm::radians(exampleRotation.x),
            vec3(1, 0, 0))  *glm::rotate(mat4(1.0f), glm::radians(exampleRotation.y),
            vec3(0, 1, 0))  *glm::rotate(mat4(1.0f), glm::radians(exampleRotation.z),
            vec3(0, 0, 1));
    mat4 expectedSm = glm::scale(vec3(exampleScale));

    object_.setPosition(examplePosition);
    object_.setRotation(exampleRotation);
    object_.setScale(exampleScale);

    /* Action */
    object_.updateModelMatrices();

    /* Validation */
    ASSERT_EQ(expectedTm, object_.getTranslationMatrix());
    ASSERT_EQ(expectedRm, object_.getRotationMatrix());
    ASSERT_EQ(expectedSm, object_.getScaleMatrix());
}

/**
 * @brief Ensures that SceneObjects will inherit model transformations when assigned a parent
 * object.
 */
TEST_F(GivenASceneObject, WhenUpdateModelMatricesWithParent_ThenUpdatedWithParentModel) {
    /* Preparation */
    vec3 examplePosition(1.0f, 2.0f, 3.0f);
    vec3 parentPosition(6.0f);
    vec3 exampleRotation(2.0f, 3.0f, 4.0f);
    vec3 parentRotation(4.0f);
    float exampleScale = 2.2f;
    float parentScale = 7.0f;

    mat4 expectedTm = glm::translate(examplePosition + parentPosition);
    mat4 expectedRm = glm::rotate(glm::radians(exampleRotation.x + parentRotation.x),
            vec3(1, 0, 0))  *glm::rotate(mat4(1.0f), glm::radians(exampleRotation.y + parentRotation.y),
            vec3(0, 1, 0))  *glm::rotate(mat4(1.0f), glm::radians(exampleRotation.z + parentRotation.z),
            vec3(0, 0, 1));
    mat4 expectedSm = glm::scale(vec3(exampleScale + parentScale));

    FakeSceneObject parentObject;
    object_.setPosition(examplePosition);
    object_.setRotation(exampleRotation);
    object_.setScale(exampleScale);

    parentObject.setPosition(parentPosition);
    parentObject.setRotation(parentRotation);
    parentObject.setScale(parentScale);

    /* Action */
    object_.setParent(&parentObject);
    object_.updateModelMatrices();

    /* Validation */
    ASSERT_EQ(expectedTm, object_.getTranslationMatrix());
    ASSERT_EQ(expectedRm, object_.getRotationMatrix());
    ASSERT_EQ(expectedSm, object_.getScaleMatrix());
}

/**
 * @brief Ensures that calling setParent connects the parent to the child, and the child to the parent.
 */
TEST_F(GivenASceneObject, WhenSetParent_ThenSceneObjectsConnected) {
    /* Preparation */
    FakeSceneObject parent(PARENT_OBJECT_NAME);

    /* Action */
    object_.setParent(&parent);

    /* Validation */
    // Make sure the fixture object is set as the single child
    ASSERT_EQ(1, parent.getChildren().size());
    ASSERT_EQ(&object_, *parent.getChildren().begin());
    // Make sure the parent is set as the fixture object's parent
    ASSERT_EQ(&parent, object_.getParent());
}

/**
 * @brief Ensures that adding a child object to a parent will connect the two objects as expected.
 */
TEST_F(GivenASceneObject, WhenAddChild_ThenSceneObjectsConnected) {
    /* Preparation */
    FakeSceneObject parent(PARENT_OBJECT_NAME);

    /* Action */
    parent.addChild(&object_);

    /* Validation */
    // Make sure the fixture object is set as the single child
    ASSERT_EQ(1, parent.getChildren().size());
    ASSERT_EQ(&object_, *parent.getChildren().begin());
    // Make sure the parent is set as the fixture object's parent
    ASSERT_EQ(&parent, object_.getParent());
}

/**
 * @brief Ensures that a child added multiple times will only be present in the set once.
 */
TEST_F(GivenASceneObject, WhenAddSameChildTwice_ThenOnlyOneInSet) {
    /* Preparation */
    FakeSceneObject parent(PARENT_OBJECT_NAME);

    /* Action */
    parent.addChild(&object_);
    parent.addChild(&object_);

    /* Validation */
    // Make sure the fixture object is set as the single child
    ASSERT_EQ(1, parent.getChildren().size());
    ASSERT_EQ(&object_, *parent.getChildren().begin());
    // Make sure the parent is set as the fixture object's parent
    ASSERT_EQ(&parent, object_.getParent());
}

class GivenASceneObjectWithParent: public ::testing::Test {
 protected:
    std::unique_ptr<FakeSceneObject> parent_;
    std::unique_ptr<FakeSceneObject> child_;
    void SetUp() override;
};

void GivenASceneObjectWithParent::SetUp() {
    // Connect the child to the parent
    parent_ = std::make_unique<FakeSceneObject>(PARENT_OBJECT_NAME);
    child_ = std::make_unique<FakeSceneObject>(TEST_OBJECT_NAME);

    parent_.get()->addChild(child_.get());
    // Make sure parent/child relationship is configured
    ASSERT_FALSE(parent_.get()->getChildren().empty());
    ASSERT_EQ(parent_.get(), child_.get()->getParent());
}

/**
 * @brief When a child object is deleted, ensure the parent no longer has a reference to it.
 */
TEST_F(GivenASceneObjectWithParent, WhenDeleteChild_ThenParentNoLongerHasChildRef) {
    /* Action */
    child_.reset();  // Delete underlying object and call destructor

    /* Validation */
    ASSERT_EQ(nullptr, child_.get());
    ASSERT_NE(nullptr, parent_.get());

    ASSERT_TRUE(parent_.get()->getChildren().empty());
}

/**
 * @brief When a parent object is deleted, ensure that the child no longer has it as its parent reference.
 */
TEST_F(GivenASceneObjectWithParent, WhenDeleteParent_ThenChildNoLongerHasParentRef) {
    /* Action */
    parent_.reset();  // Delete underlying object and call destructor

    /* Validation */
    ASSERT_NE(nullptr, child_.get());
    ASSERT_EQ(nullptr, parent_.get());

    ASSERT_EQ(nullptr, child_.get()->getParent());
}
