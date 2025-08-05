cmake_minimum_required(VERSION 3.16)
project(awa5_interpreter LANGUAGES CXX)

target_compile_features(awa5_interpreter cxx_std_20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(awa5_interpreter main.cpp)