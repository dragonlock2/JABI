cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(jabi)

target_include_directories(app PRIVATE inc/)

FILE(GLOB_RECURSE app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})

# can_mcan.c filters only work on ID and ID type and rest is done in software
# this is an issue if we want to receive FD and classic packets since one gets dropped
add_custom_target(
    apply_jabi_patch
    COMMAND git apply ${CMAKE_CURRENT_LIST_DIR}/zephyr.patch
    WORKING_DIRECTORY $ENV{ZEPHYR_BASE}
)
add_dependencies(version_h apply_jabi_patch)

add_custom_target(
    remove_jabi_patch ALL
    COMMAND git apply -R ${CMAKE_CURRENT_LIST_DIR}/zephyr.patch
    WORKING_DIRECTORY $ENV{ZEPHYR_BASE}
    DEPENDS zephyr_final
)
