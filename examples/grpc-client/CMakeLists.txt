cmake_minimum_required(VERSION 3.20.0)

project(grpc-client)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(MSVC)
    find_library(GETOPT_LIBRARIES NAMES getopt)
    # add_compile_options(/W4 /WX) # gRPC warnings not suppressed?
else()
    add_compile_options(-Wall -Wextra -Werror)
endif()

add_subdirectory(../../include/protos ${CMAKE_CURRENT_BINARY_DIR}/protos)
add_executable(main
    main.cpp
    client.cpp
)

target_link_libraries(main jabiproto ${GETOPT_LIBRARIES})
