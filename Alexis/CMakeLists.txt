cmake_minimum_required(VERSION 3.1)

# name of the project
project(DataGenerator VERSION 2.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include src)
set(SOURCE_FILES main.cpp include/simple_file_parser/simple_file_parser.h include/simple_file_parser/simple_file_parser.cc src/Attribute.h src/Row.h )#src/Utils.cpp src/Utils.h)

# create the executable
add_executable(DataGenerator ${SOURCE_FILES})

