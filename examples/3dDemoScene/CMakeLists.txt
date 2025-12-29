cmake_minimum_required(VERSION 3.16)

project(studious-3dExampleScene VERSION 1.0.0)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(studious-3dExampleScene
	game.cpp
	inputMonitor.cpp
)

target_link_libraries(studious-3dExampleScene
	studious
)

install(TARGETS studious-3dExampleScene)