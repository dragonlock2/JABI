cmake_minimum_required(VERSION 3.20.0)

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

add_library(jabiproto
    jabi.proto
)

# gRPC generates a bunch of warnings, suppressing them
target_include_directories(jabiproto SYSTEM PUBLIC
    ${Protobuf_INCLUDE_DIRS}
    ${gRPC_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(jabiproto PUBLIC
    gRPC::grpc++
)

get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)
protobuf_generate(TARGET jabiproto LANGUAGE cpp)
protobuf_generate(TARGET jabiproto LANGUAGE grpc 
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}"
)
