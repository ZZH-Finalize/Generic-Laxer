cmake_minimum_required(VERSION 3.21)
project(edl-laxer LANGUAGES CXX VERSION 0.0.1)
set (CMAKE_USE_RELATIVE_PATHS ON)

include_directories(
    .
)

add_definitions(-D__OPT_DEBUG)

add_library(laxer SHARED
    laxer.cpp
)

add_executable(${PROJECT_NAME}
    main.cpp
)

target_link_libraries(${PROJECT_NAME} laxer)
