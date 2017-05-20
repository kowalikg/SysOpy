cmake_minimum_required(VERSION 3.7)
project(a)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pthread")
set(SOURCE_FILES main.c)
add_executable(a ${SOURCE_FILES})