cmake_minimum_required(VERSION 3.20.0)
project(pyjabi)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(pybind11)
pybind11_add_module(jabi jabi.cpp) # defines the import name

add_subdirectory(../libjabi ${CMAKE_CURRENT_BINARY_DIR}/libjabi)
target_link_libraries(jabi PRIVATE libjabi)
set_target_properties(libjabi PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
