cmake_minimum_required(VERSION 3.12)

add_executable(fiber "main.cpp")

target_include_directories(fiber PRIVATE lib)
target_include_directories(fiber PRIVATE lib/execution)
target_include_directories(fiber PRIVATE lib/interprete)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT fiber)

set(examplesDirectory ${CMAKE_CURRENT_SOURCE_DIR}/../example)

set_target_properties(fiber PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${examplesDirectory})
