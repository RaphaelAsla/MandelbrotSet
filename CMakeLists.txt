cmake_minimum_required(VERSION 3.16)

project(AVA LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Werror -O3")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(fractal)

target_sources(fractal
PUBLIC
	main.cpp
)

target_compile_options(fractal PRIVATE -Wall -Wextra -Wpedantic)

target_link_libraries(fractal -lsfml-graphics -lsfml-window -lsfml-system)
