cmake_minimum_required(VERSION 3.28)
project(jayce C)

set(CMAKE_C_STANDARD 11)

add_executable(jayce 
    main.c
    src/deck.c
    src/gamestate.c
    src/rng.c
)

target_include_directories(jayce PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
