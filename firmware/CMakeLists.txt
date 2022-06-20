cmake_minimum_required(VERSION 3.20.0)

set(CONF_FILE prj.conf)
list(APPEND CONF_FILE boards/${BOARD}.conf)
set(DTC_OVERLAY_FILE boards/${BOARD}.overlay)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(jabi)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
