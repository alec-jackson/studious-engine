cmake_minimum_required(VERSION 3.16)

project(studious-2dExampleScene VERSION 1.0.0)

add_executable(studious-2dExampleScene
	game.cpp
)

target_link_libraries(studious-2dExampleScene
	studious
)

install(TARGETS studious-2dExampleScene)