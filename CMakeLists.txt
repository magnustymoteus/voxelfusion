cmake_minimum_required(VERSION 3.22)
project(voxelfusion)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-Werror -Wall")

file(GLOB_RECURSE LIB src/lib/*.cpp)
file(GLOB_RECURSE CFG src/CFG/*.cpp)
file(GLOB_RECURSE TM src/MTMDTuringMachine/*cpp)
file(GLOB_RECURSE PARSER src/Parser/*.cpp)

include_directories(src)

ADD_EXECUTABLE(voxelfusion src/main.cpp ${LIB} ${CFG} ${TM} ${PARSER})