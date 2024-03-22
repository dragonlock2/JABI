cmake_minimum_required(VERSION 3.20.0)

project(grpc-server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
    find_library(GETOPT_LIBRARIES NAMES getopt)
    # add_compile_options(/W4 /WX) # gRPC warnings not suppressed?
else()
    add_compile_options(-Wall -Wextra -Werror)
endif()

add_subdirectory(../../include/protos ${CMAKE_CURRENT_BINARY_DIR}/protos)
add_subdirectory(../cpp ${CMAKE_CURRENT_BINARY_DIR}/cpp)
add_executable(main
    main.cpp
    server.cpp
)

target_link_libraries(main jabiproto jabi ${GETOPT_LIBRARIES})
