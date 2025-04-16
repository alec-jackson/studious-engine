cmake_minimum_required(VERSION 3.16)

enable_testing()
project(engineExample VERSION 0.0.1)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if (GFX_EMBEDDED)
  add_compile_options(-DGFX_EMBEDDED)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT WIN32)
add_compile_options(-Wall -Werror)
endif()

if (APPLE)
include_directories(
  /usr/local/Cellar/glew/2.2.0_1/include/
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
  src/main/example/headers
  src/main/misc/headers
)

include(GoogleTest)
include(CTest)

find_package(GLEW 2.0 REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_image REQUIRED)
#find_package(SDL2_net REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
find_package(Freetype REQUIRED)
find_package(GTest CONFIG)

# Add additional games here :)
if (MAIN_TARGET STREQUAL "default")
  set(MAIN_FILE 
    src/main/example/src/game.cpp
  )
elseif (MAIN_TARGET STREQUAL "chris_2d")
  set(MAIN_FILE 
    src/main/example/2dDemo/game.cpp
  )
else()
  message("Unknown main target: ${MAIN_TARGET}. Please ensure the MAIN_TARGET is valid!")
  message("Defaulting to 'default' target.")
  set(MAIN_FILE 
    src/main/example/src/game.cpp
  )
endif()

add_executable(${PROJECT_NAME}
  ${MAIN_FILE}
  src/main/utilities/src/ModelImport.cpp
  src/main/utilities/src/Polygon.cpp
  src/main/engine/Misc/src/GameInstance.cpp
  src/main/engine/SceneObject/src/GameObject.cpp
  src/main/engine/SceneObject/src/SpriteObject.cpp
  src/main/engine/SceneObject/src/UiObject.cpp
  src/main/engine/SceneObject/src/TextObject.cpp
  src/main/engine/SceneObject/src/GameObject2D.cpp
  src/main/engine/SceneObject/src/CameraObject.cpp
  src/main/engine/SceneObject/src/ColliderObject.cpp
  src/main/utilities/src/GifLoader.cpp
  src/main/engine/Misc/src/inputMonitor.cpp
  src/main/engine/Misc/src/physics.cpp
  src/main/engine/GfxController/src/DummyGfxController.cpp
  src/main/engine/GfxController/src/OpenGlGfxController.cpp
  src/main/engine/GfxController/src/OpenGlEsGfxController.cpp
  src/main/engine/AnimationController/src/AnimationController.cpp
  src/main/misc/src/config.cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

# Project include dirs

target_include_directories(${PROJECT_NAME}
  PUBLIC ${SDL2_INCLUDE_DIRS}
  PUBLIC ${GLEW_INCLUDE_DIRS}
  PUBLIC ${OPENGL_INCLUDE_DIR}
  PUBLIC ${FREETYPE_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC SDL2::Main
  PUBLIC SDL2::Mixer
  PUBLIC SDL2::Image
  PUBLIC ${OPENGL_LIBRARIES}
  PUBLIC ${GLEW_LIBRARIES}
  PUBLIC Threads::Threads
  PUBLIC ${FREETYPE_LIBRARIES}
)

set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

if (GTest_FOUND)
# ======================================== ModelImportTests ========================================
add_executable(gtest_ModelImportTests
  src/main/utilities/test/src/ModelImportTests.cpp
  src/main/utilities/src/ModelImport.cpp
  src/main/utilities/src/Polygon.cpp
)

target_include_directories(gtest_ModelImportTests
  PUBLIC ${SDL2_INCLUDE_DIRS}
  PUBLIC ${GLEW_INCLUDE_DIRS}
  PUBLIC ${OPENGL_INCLUDE_DIR}
  PUBLIC ${FREETYPE_INCLUDE_DIRS}
  PRIVATE src/main/utilities/test/headers
)

target_link_libraries(gtest_ModelImportTests
  PUBLIC SDL2::Main
  PUBLIC SDL2::Mixer
  PUBLIC SDL2::Image
  PUBLIC ${OPENGL_LIBRARIES}
  PUBLIC ${GLEW_LIBRARIES}
  PUBLIC Threads::Threads
  PUBLIC ${FREETYPE_LIBRARIES}
  PUBLIC GTest::gtest_main
)

gtest_discover_tests(gtest_ModelImportTests)

# ======================================== PolygonTests ========================================
add_executable(gtest_PolygonTests
  src/main/utilities/test/src/PolygonTests.cpp
  src/main/engine/GfxController/src/DummyGfxController.cpp
  src/main/utilities/src/Polygon.cpp
)

target_include_directories(gtest_PolygonTests
  PRIVATE src/main/utilities/test/headers
  PUBLIC ${SDL2_INCLUDE_DIRS}
  PUBLIC ${GLEW_INCLUDE_DIRS}
  PUBLIC ${OPENGL_INCLUDE_DIR}
  PUBLIC ${FREETYPE_INCLUDE_DIRS}
)

target_link_libraries(gtest_PolygonTests 
  PUBLIC SDL2::Main
  PUBLIC SDL2::Mixer
  PUBLIC SDL2::Image
  PUBLIC ${OPENGL_LIBRARIES}
  PUBLIC ${GLEW_LIBRARIES}
  PUBLIC Threads::Threads
  PUBLIC ${FREETYPE_LIBRARIES}
  PUBLIC GTest::gtest_main
)

gtest_discover_tests(gtest_PolygonTests)

# ======================================== SpriteObjectTests ========================================
add_executable(gtest_SpriteObjectTests
  src/main/engine/SceneObject/test/src/SpriteObjectTests.cpp
  src/main/engine/SceneObject/src/SpriteObject.cpp
  src/main/engine/SceneObject/src/GameObject2D.cpp
  src/main/engine/SceneObject/src/ColliderObject.cpp
  src/main/utilities/src/Polygon.cpp
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
# ======================================== AnimationControllerTests ========================================
add_executable(gtest_AnimationControllerTests
  src/main/engine/AnimationController/test/src/AnimationControllerTests.cpp
  src/main/engine/AnimationController/src/AnimationController.cpp
  src/main/engine/SceneObject/src/SpriteObject.cpp
  src/main/engine/SceneObject/src/GameObject2D.cpp
  src/main/engine/SceneObject/src/ColliderObject.cpp
  src/main/engine/SceneObject/src/TextObject.cpp
  src/main/engine/SceneObject/src/UiObject.cpp
  src/main/utilities/src/Polygon.cpp
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
  GTest::gmock
  PUBLIC ${FREETYPE_LIBRARIES}
)

gtest_discover_tests(gtest_AnimationControllerTests)
# ======================================== END OF GTESTS ========================================
endif()

install(TARGETS ${PROJECT_NAME})
