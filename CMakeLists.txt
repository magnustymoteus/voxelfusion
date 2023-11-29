cmake_minimum_required(VERSION 3.22)
project(voxelfusion)
cmake_policy(SET CMP0072 NEW)

set(CMAKE_CXX_STANDARD 20)
#set(CMAKE_CXX_FLAGS "-Werror -Wall") TODO circumvent errors in glm/json.hpp?

file(GLOB_RECURSE LIB src/lib/*.cpp)
file(GLOB_RECURSE CFG src/CFG/*.cpp)
file(GLOB_RECURSE TM src/MTMDTuringMachine/*.cpp )
file(GLOB_RECURSE PARSER src/LR1Parser/*.cpp)
file(GLOB_RECURSE LEXER src/Lexer/*.cpp)
file(GLOB_RECURSE VISUALISATION src/Visualisation/*.cpp)
file(GLOB_RECURSE UTILS src/utils/*.cpp src/utils/*.cc)
list(APPEND VISUALISATION src/lib/glad/src/glad.c)

include_directories(src src/lib src/lib/glad/include src/utils)

if (WIN32)
    list(APPEND CMAKE_PREFIX_PATH "src/lib/GLFW/lib/cmake")
endif (WIN32)

find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)

ADD_EXECUTABLE(voxelfusion src/main.cpp ${LIB} ${CFG} ${TM} ${LEXER} ${PARSER} ${VISUALISATION} ${UTILS})

target_link_libraries(voxelfusion glfw OpenGL::GL)