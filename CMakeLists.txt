cmake_minimum_required(VERSION 3.17)
project(SFML_TextBox)

set(CMAKE_CXX_STANDARD 17)

add_library(SFML_TextBox TextBox.cpp TextBox.hpp)
add_subdirectory(SFML)
target_link_libraries(SFML_TextBox sfml-graphics)