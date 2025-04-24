cmake_minimum_required(VERSION 3.16)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)

set(CMAKE_CXX_FLAGS_RELEASE "-O3")
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

project(yourProjectName VERSION 1.0.0)

find_package(studious REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME}
	gameTemplate.cpp
)

target_link_libraries(${PROJECT_NAME}
	studious::studious
)
