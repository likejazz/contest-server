cmake_minimum_required(VERSION 2.8.4)
project(contest)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(SOURCE_FILES contest.c client.c error.c)

add_executable(contest ${SOURCE_FILES})