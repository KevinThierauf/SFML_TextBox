cmake_minimum_required(VERSION 3.17)
project(SFML_TextBox)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

add_library(SFML_TextBox STATIC TextBox.cpp TextBox.hpp ScrollBar.hpp ScrollBar.cpp Reference.hpp TextStyle.hpp InputHandler.hpp Pos.hpp CaretStyle.hpp Caret.hpp Caret.cpp CaretStyle.cpp Pos.cpp InputHandler.cpp CharPos.hpp CharPos.cpp)
add_subdirectory(SFML)
target_link_libraries(SFML_TextBox sfml-graphics)

option(BUILD_DEMOS "build demo programs" ON)
if (${BUILD_DEMOS})
    add_subdirectory(demos)
endif ()