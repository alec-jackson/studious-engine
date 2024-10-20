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

add_compile_options(-Wall -Werror)

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

add_executable(${PROJECT_NAME}
  src/main/example/src/game.cpp
  src/main/utilities/src/ModelImport.cpp
  src/main/utilities/src/Polygon.cpp
  src/main/engine/Misc/src/GameInstance.cpp
  src/main/engine/SceneObject/src/GameObject.cpp
  src/main/engine/SceneObject/src/SpriteObject.cpp
  src/main/engine/SceneObject/src/UiObject.cpp
  src/main/engine/SceneObject/src/TextObject.cpp
  src/main/engine/SceneObject/src/CameraObject.cpp
  src/main/engine/SceneObject/src/ColliderObject.cpp
  src/main/engine/Misc/src/inputMonitor.cpp
  src/main/engine/Misc/src/physics.cpp
  src/main/engine/GfxController/src/DummyGfxController.cpp
  src/main/engine/GfxController/src/OpenGlGfxController.cpp
  src/main/engine/GfxController/src/OpenGlEsGfxController.cpp
  src/main/misc/src/config.cpp
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

IF($(GTest_FOUND))
add_executable(gtest_ModelImportTests
  src/main/utilities/test/src/ModelImportTests.cpp
  src/main/utilities/src/ModelImport.cpp
  src/main/utilities/src/Polygon.cpp
  src/main/engine/GfxController/src/DummyGfxController.cpp
)

add_executable(gtest_PolygonTests
  src/main/utilities/test/src/PolygonTests.cpp
  src/main/engine/GfxController/src/DummyGfxController.cpp
  src/main/utilities/src/Polygon.cpp
)

target_include_directories(gtest_ModelImportTests
  PRIVATE src/main/utilities/test/headers
)

target_include_directories(gtest_PolygonTests
  PRIVATE src/main/utilities/test/headers
)

target_include_directories(gtest_ModelImportTests PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(gtest_ModelImportTests PUBLIC ${GLEW_INCLUDE_DIRS})
target_include_directories(gtest_ModelImportTests PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(gtest_ModelImportTests PUBLIC ${FREETYPE_INCLUDE_DIRS})
#target_include_directories(gtest_ModelImportTests PUBLIC ${SDL2_NET_INCLUDE_DIRS})

target_include_directories(gtest_PolygonTests PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(gtest_PolygonTests PUBLIC ${GLEW_INCLUDE_DIRS})
target_include_directories(gtest_PolygonTests PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(gtest_PolygonTests PUBLIC ${FREETYPE_INCLUDE_DIRS})
#target_include_directories(gtest_PolygonTests PUBLIC ${SDL2_NET_INCLUDE_DIRS})

target_link_libraries(gtest_ModelImportTests SDL2::Main)
target_link_libraries(gtest_ModelImportTests SDL2::Mixer)
target_link_libraries(gtest_ModelImportTests SDL2::Image)
#target_link_libraries(gtest_ModelImportTests SDL2::Net)
target_link_libraries(gtest_ModelImportTests ${OPENGL_LIBRARIES})

target_link_libraries(gtest_ModelImportTests ${GLEW_LIBRARIES})
target_link_libraries(gtest_ModelImportTests Threads::Threads)
target_link_libraries(gtest_ModelImportTests ${FREETYPE_LIBRARIES})

target_link_libraries(gtest_PolygonTests SDL2::Main)
target_link_libraries(gtest_PolygonTests SDL2::Mixer)
target_link_libraries(gtest_PolygonTests SDL2::Image)
#target_link_libraries(gtest_PolygonTests SDL2::Net)
target_link_libraries(gtest_PolygonTests ${OPENGL_LIBRARIES})
target_link_libraries(gtest_PolygonTests ${GLEW_LIBRARIES})
target_link_libraries(gtest_PolygonTests Threads::Threads)
target_link_libraries(gtest_PolygonTests ${FREETYPE_LIBRARIES})

target_link_libraries(
  gtest_ModelImportTests
  GTest::gtest_main
)

target_link_libraries(
  gtest_PolygonTests
  GTest::gtest_main
)

gtest_discover_tests(gtest_PolygonTests)
gtest_discover_tests(gtest_ModelImportTests)
ENDIF($(GTest_FOUND))

# Project include dirs
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 14)
target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${GLEW_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PUBLIC ${FREETYPE_INCLUDE_DIRS})
#target_include_directories(${PROJECT_NAME} PUBLIC ${SDL2_NET_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} SDL2::Main)
target_link_libraries(${PROJECT_NAME} SDL2::Mixer)
target_link_libraries(${PROJECT_NAME} SDL2::Image)
#target_link_libraries(${PROJECT_NAME} SDL2::Net)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARIES})
target_link_libraries(${PROJECT_NAME} Threads::Threads)
target_link_libraries(${PROJECT_NAME} ${FREETYPE_LIBRARIES})

set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
