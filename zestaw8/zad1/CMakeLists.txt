cmake_minimum_required(VERSION 3.7)
project(zad1)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pthread")
set(SOURCE_FILES fileGenerator.c threadSimulator2.c)
add_executable(zad1 ${SOURCE_FILES})