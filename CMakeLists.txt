cmake_minimum_required(VERSION 2.8)
project(DataGenerator)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(include)
set(SOURCE_FILES main.cpp include/simple_file_parser/simple_file_parser.h include/simple_file_parser/simple_file_parser.cc src/Attribute.h src/Attribute.cpp src/Row.cpp src/Row.h src/Utils.cpp src/Utils.h)
add_executable(DataGenerator ${SOURCE_FILES})

