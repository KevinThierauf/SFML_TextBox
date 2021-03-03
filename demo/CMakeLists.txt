add_executable(Demo main.cpp)
target_include_directories(Demo PRIVATE ../SFML/include)
target_include_directories(Demo PRIVATE ..)
target_link_libraries(Demo SFML_TextBox)
