cmake_minimum_required(VERSION 3.16)

enable_testing()
project(studious VERSION 1.0.0)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if (GFX_EMBEDDED)
  add_compile_options(-DGFX_EMBEDDED)
endif()
if (PHYS_THREADS)
  add_compile_options(-DPHYS_THREADS=${PHYS_THREADS})
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT WIN32)
add_compile_options(-Wall -Werror -Wextra)
endif()

if (APPLE)
include_directories(
  /usr/local/Cellar/glm/0.9.9.8/include/
)
endif (APPLE)

include_directories(
  src/main/utilities/headers
  src/main/common/headers
  src/main/engine/GfxController/headers
  src/main/engine/AnimationController/headers
  src/main/engine/Misc/headers
  src/main/engine/SceneObject/headers
  src/main/engine/SceneObjectExt/headers
  src/main/misc/headers
  src/main/opengl/headers
)

include(GoogleTest)
include(CTest)

find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_image REQUIRED)
#find_package(SDL2_net REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
find_package(Freetype REQUIRED)
find_package(GTest CONFIG)


if(EXAMPLES)

  add_subdirectory(examples/2dDemo)
  add_subdirectory(examples/3dDemoScene)

endif()

add_library(${PROJECT_NAME} SHARED
  src/main/utilities/src/ModelImport.cpp
  src/main/engine/Misc/src/GameInstance.cpp
  src/main/engine/Misc/src/GameScene.cpp
  src/main/engine/SceneObject/src/SceneObject.cpp
  src/main/engine/SceneObject/src/GameObject.cpp
  src/main/engine/SceneObject/src/SpriteObject.cpp
  src/main/engine/SceneObject/src/UiObject.cpp
  src/main/engine/SceneObject/src/TextObject.cpp
  src/main/engine/SceneObject/src/GameObject2D.cpp
  src/main/engine/SceneObject/src/CameraObject.cpp
  src/main/engine/SceneObject/src/TPSCameraObject.cpp
  src/main/engine/SceneObject/src/FPSCameraObject.cpp
  src/main/engine/SceneObject/src/ColliderObject.cpp
  src/main/engine/SceneObject/src/TileObject.cpp
  src/main/engine/SceneObjectExt/src/TrackExt.cpp
  src/main/engine/SceneObjectExt/src/ImageExt.cpp
  src/main/engine/SceneObjectExt/src/ColliderExt.cpp
  src/main/utilities/src/GifLoader.cpp
  src/main/engine/Misc/src/physics.cpp
  src/main/engine/GfxController/src/DummyGfxController.cpp
  src/main/engine/GfxController/src/OpenGlGfxController.cpp
  src/main/engine/AnimationController/src/AnimationController.cpp
  src/main/engine/Misc/src/InputController.cpp
  src/main/engine/Misc/src/DeltaTime.cpp
  src/main/engine/Misc/src/Image.cpp
  src/main/misc/src/config.cpp
  src/main/opengl/src/es/glad.c
  src/main/opengl/src/core/glad.c
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

# Project include dirs

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/studious>
  PUBLIC ${SDL2_INCLUDE_DIRS}
  PUBLIC ${OPENGL_INCLUDE_DIR}
  PUBLIC ${FREETYPE_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC SDL2::Main
  PUBLIC SDL2::Mixer
  PUBLIC SDL2::Image
  PUBLIC ${OPENGL_LIBRARIES}
  PUBLIC Threads::Threads
  PUBLIC ${FREETYPE_LIBRARIES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

if (GTest_FOUND AND RUNTEST)
# ======================================== ModelImportTests ========================================
add_executable(gtest_ModelImportTests
  src/main/utilities/test/src/ModelImportTests.cpp
  src/main/utilities/src/ModelImport.cpp
)

target_include_directories(gtest_ModelImportTests
  PUBLIC ${SDL2_INCLUDE_DIRS}
  PUBLIC ${OPENGL_INCLUDE_DIR}
  PUBLIC ${FREETYPE_INCLUDE_DIRS}
  PRIVATE src/main/utilities/test/headers
)

target_link_libraries(gtest_ModelImportTests
  PUBLIC SDL2::Main
  PUBLIC SDL2::Mixer
  PUBLIC SDL2::Image
  PUBLIC ${OPENGL_LIBRARIES}
  PUBLIC Threads::Threads
  PUBLIC ${FREETYPE_LIBRARIES}
  PUBLIC GTest::gtest_main
)

gtest_discover_tests(gtest_ModelImportTests)

# ======================================== SpriteObjectTests ========================================
add_executable(gtest_SpriteObjectTests
  src/main/engine/SceneObject/test/src/SpriteObjectTests.cpp
  src/main/engine/SceneObject/src/SceneObject.cpp
  src/main/engine/SceneObject/src/SpriteObject.cpp
  src/main/engine/SceneObject/src/GameObject2D.cpp
  src/main/engine/SceneObject/src/ColliderObject.cpp
  src/main/engine/Misc/src/Image.cpp
  src/main/engine/SceneObjectExt/src/TrackExt.cpp
  src/main/engine/SceneObjectExt/src/ImageExt.cpp
  src/main/engine/SceneObjectExt/src/ColliderExt.cpp
)

target_include_directories(gtest_SpriteObjectTests
  PRIVATE src/main/engine/SceneObject/test/headers
  PRIVATE src/main/engine/SceneObject/headers
)

target_link_libraries(gtest_SpriteObjectTests
  PUBLIC SDL2::Image
  GTest::gtest_main
  GTest::gmock
)

gtest_discover_tests(gtest_SpriteObjectTests)

# ======================================== SceneObjectTests ========================================
add_executable(gtest_SceneObjectTests
  src/main/engine/SceneObject/test/SceneObjectTests.cpp
  src/main/engine/SceneObject/src/SceneObject.cpp
  src/main/engine/SceneObject/src/TestObject.cpp
  src/main/engine/GfxController/src/DummyGfxController.cpp
  src/main/engine/SceneObject/src/ColliderObject.cpp
)

target_include_directories(gtest_SceneObjectTests
  PUBLIC ${SDL2_INCLUDE_DIRS}
  PRIVATE src/main/engine/SceneObject/test/headers
  PRIVATE src/main/engine/SceneObject/headers
)

target_link_libraries(gtest_SceneObjectTests
  GTest::gtest_main
  GTest::gmock
)

gtest_discover_tests(gtest_SceneObjectTests)
# ======================================== AnimationControllerTests ========================================
add_executable(gtest_AnimationControllerTests
  src/main/engine/AnimationController/test/src/AnimationControllerTests.cpp
  src/main/engine/AnimationController/src/AnimationController.cpp
  src/main/engine/SceneObject/src/TestObject.cpp
  src/main/engine/SceneObject/src/SceneObject.cpp
  src/main/engine/SceneObject/src/SpriteObject.cpp
  src/main/engine/SceneObject/src/GameObject2D.cpp
  src/main/engine/SceneObject/src/ColliderObject.cpp
  src/main/engine/SceneObject/src/TextObject.cpp
  src/main/engine/SceneObject/src/UiObject.cpp
  src/main/engine/GfxController/src/DummyGfxController.cpp
  src/main/engine/Misc/src/DeltaTime.cpp
  src/main/engine/SceneObjectExt/src/TrackExt.cpp
  src/main/engine/SceneObjectExt/src/ImageExt.cpp
  src/main/engine/SceneObjectExt/src/ColliderExt.cpp
  src/main/engine/Misc/src/Image.cpp
)

target_include_directories(gtest_AnimationControllerTests
  PRIVATE src/main/engine/AnimationController/test/headers
  PRIVATE src/main/engine/AnimationController/headers
  PUBLIC ${FREETYPE_INCLUDE_DIRS}
)

target_link_libraries(gtest_AnimationControllerTests
  PUBLIC SDL2::Image
  GTest::gtest_main
  GTest::gmock
  PUBLIC ${FREETYPE_LIBRARIES}
)

gtest_discover_tests(gtest_AnimationControllerTests)
# ======================================== PhysicsControllerTests ========================================
add_executable(gtest_PhysicsControllerTests
  src/main/engine/Misc/test/src/PhysicsControllerTests.cpp
  src/main/engine/Misc/src/physics.cpp
  src/main/engine/Misc/src/DeltaTime.cpp
  src/main/engine/SceneObject/src/TestObject.cpp
  src/main/engine/SceneObject/src/SceneObject.cpp
  src/main/engine/SceneObject/src/ColliderObject.cpp
  src/main/engine/SceneObjectExt/src/ColliderExt.cpp
  src/main/engine/GfxController/src/DummyGfxController.cpp
)

target_include_directories(gtest_PhysicsControllerTests
  PRIVATE
    src/main/engine/Misc/test/headers
  PUBLIC
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(gtest_PhysicsControllerTests
  PUBLIC
  GTest::gtest_main
  GTest::gmock
  ${FREETYPE_LIBRARIES}
)

gtest_discover_tests(gtest_PhysicsControllerTests)
# ======================================== END OF GTESTS ========================================
endif()

# --------------------------------------- LIBRARY INSTALL --------------------------------------

install(TARGETS ${PROJECT_NAME}
    EXPORT studiousTargets  # Add the target to the export group
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install (FILES
  src/main/utilities/headers/ModelImport.hpp
  src/main/utilities/headers/Polygon.hpp
  src/main/utilities/headers/Model.hpp
  src/main/utilities/headers/Material.hpp
  src/main/common/headers/common.hpp
  src/main/common/headers/winsup.hpp
  src/main/engine/SceneObject/headers/CameraObject.hpp
  src/main/engine/SceneObject/headers/TPSCameraObject.hpp
  src/main/engine/SceneObject/headers/ColliderObject.hpp
  src/main/engine/SceneObject/headers/GameObject.hpp
  src/main/engine/SceneObject/headers/GameObject2D.hpp
  src/main/engine/SceneObject/headers/SceneObject.hpp
  src/main/engine/SceneObject/headers/SpriteObject.hpp
  src/main/engine/SceneObject/headers/TextObject.hpp
  src/main/engine/SceneObject/headers/UiObject.hpp
  src/main/engine/SceneObject/headers/TileObject.hpp
  src/main/engine/SceneObjectExt/headers/TrackExt.hpp
  src/main/engine/SceneObjectExt/headers/ImageExt.hpp
  src/main/engine/SceneObjectExt/headers/ColliderExt.hpp
  src/main/engine/GfxController/headers/GfxController.hpp
  src/main/engine/GfxController/headers/OpenGlGfxController.hpp
  src/main/engine/Misc/headers/GameInstance.hpp
  src/main/engine/Misc/headers/InputController.hpp
  src/main/engine/Misc/headers/GameScene.hpp
  src/main/engine/Misc/headers/Image.hpp
  src/main/engine/Misc/headers/physics.hpp
  src/main/misc/headers/config.hpp
  src/main/engine/AnimationController/headers/AnimationController.hpp
  DESTINATION include/studious
)

install (DIRECTORY
  src/main/opengl/headers/core
  src/main/opengl/headers/es
  DESTINATION include/studious
)

install(EXPORT studiousTargets
    FILE studious-config.cmake
    NAMESPACE studious::
    DESTINATION lib/cmake/studious
)
