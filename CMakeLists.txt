cmake_minimum_required(VERSION 3.15)
project(SelfUtilMacOS)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-macro-redefined")

include_directories(${CMAKE_SOURCE_DIR})

add_executable(selfutil
    selfutil.cpp
)
