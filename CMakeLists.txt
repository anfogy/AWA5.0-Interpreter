cmake_minimum_required(VERSION 3.16)
project(awa5_interpreter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(awa5_interpreter main.cpp)