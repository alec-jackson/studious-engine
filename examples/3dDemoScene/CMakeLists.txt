cmake_minimum_required(VERSION 3.16)

project(studious-3dExampleScene VERSION 1.0.0)

add_executable(studious-3dExampleScene
	game.cpp
)

target_link_libraries(studious-3dExampleScene
	studious
)

install(TARGETS studious-3dExampleScene)