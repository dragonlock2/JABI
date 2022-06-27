cmake_minimum_required(VERSION 3.20.0)

project(test_cpplib)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror)
endif()

add_subdirectory(../../clients/libjabi ${CMAKE_CURRENT_BINARY_DIR}/libjabi)

add_executable(main main.cpp)

target_link_libraries(main libjabi)
