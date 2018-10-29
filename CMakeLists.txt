cmake_minimum_required(VERSION 2.8)

project(MyBot)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wno-unused-function -pedantic")

include_directories( ${CMAKE_SOURCE_DIR}/src)

get_property(HLT_DIR DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)

file(GLOB_RECURSE SOURCE ${CMAKE_SOURCE_DIR}/hlt/*.[ch]*)
set(HLT_SOURCE_FILES "${SOURCE_FILES}" ${SOURCE})
add_library(hlt ${HLT_SOURCE_FILES})

file(GLOB_RECURSE SOURCE${CMAKE_SOURCE_DIR}/src/*.[ch]*)
set(AI_SOURCE_FILES "${SOURCE_FILES}" ${SOURCE})
add_library(ai ${AI_SOURCE_FILES})

include_directories(${CMAKE_SOURCE_DIR})
add_executable(MyBot MyBot.cpp)
target_link_libraries(MyBot ai hlt)


if(MINGW)
    target_link_libraries(MyBot -static)
endif()
