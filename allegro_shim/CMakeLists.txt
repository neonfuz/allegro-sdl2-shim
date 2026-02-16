cmake_minimum_required(VERSION 3.10)
project(allegro_shim VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_ttf REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SDL2_INCLUDE_DIRS}
    ${SDL2_IMAGE_INCLUDE_DIRS}
    ${SDL2_MIXER_INCLUDE_DIRS}
    ${SDL2_TTF_INCLUDE_DIRS}
)

set(SOURCES
    src/allegro_shim.cpp
)

add_library(allegro_shim STATIC ${SOURCES})

target_link_libraries(allegro_shim
    SDL2::SDL2
    SDL2::SDL2_image
    SDL2::SDL2_mixer
    SDL2::SDL2_ttf
)
