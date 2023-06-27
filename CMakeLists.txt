cmake_minimum_required(VERSION 3.16)

project(engineExample VERSION 0.0.1)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if (APPLE)
include_directories(
  /usr/local/Cellar/glew/2.2.0_1/include/
  /usr/local/Cellar/glm/0.9.9.8/include/
)
endif (APPLE)

include_directories(
  src/main/utilities/headers
  src/main/common/headers
  src/main/engine/headers
  src/main/example/headers
  src/main/misc/headers
)

find_package(GLEW 2.0 REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_image REQUIRED)
#find_package(SDL2_net REQUIRED)
find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)
find_package(Freetype REQUIRED)

add_executable(${PROJECT_NAME}
  src/main/example/src/game.cpp
  src/main/utilities/src/ModelImport.cpp
  src/main/utilities/src/shaderLoader.cpp
  src/main/engine/src/gameInstance.cpp
  src/main/engine/src/gameObject.cpp
  src/main/engine/src/inputMonitor.cpp
  src/main/engine/src/physics.cpp
  src/main/misc/src/config.cpp
  src/main/misc/src/loadScene.cpp
)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 11)
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
