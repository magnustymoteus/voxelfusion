include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.12.0
)
FetchContent_MakeAvailable(googletest)
add_library(GTest::GTest INTERFACE IMPORTED)
target_link_libraries(GTest::GTest INTERFACE gtest_main)

add_executable(test test.cpp ${CFG} ${TM} ${LEXER} ${PARSER} ${UTILS} ${TM_GENERATION} ${OBJ_PARSER})

target_link_libraries(test
        PRIVATE
        GTest::GTest)