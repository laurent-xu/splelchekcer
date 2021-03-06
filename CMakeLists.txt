cmake_minimum_required(VERSION 2.6)
project (textmining)

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

if(EXISTS "${CMAKE_MODULE_PATH}/common.cmake")
    include(common)
else()
    message(FATAL_ERROR "Run ./configure")
endif()

include_directories(src)

# Main binaries

add_executable (TextMiningCompiler src/compiler.cc)
add_executable (TextMiningApp src/app.cc)

set_target_properties(TextMiningCompiler PROPERTIES RUNTIME_OUTPUT_DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(TextMiningApp PROPERTIES RUNTIME_OUTPUT_DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable (SerializerTool EXCLUDE_FROM_ALL tests/serialize_tool.cc)
set_target_properties(SerializerTool PROPERTIES RUNTIME_OUTPUT_DIRECTORY
    "${CMAKE_CURRENT_SOURCE_DIR}/tests")
add_custom_target(tools DEPENDS SerializerTool)
