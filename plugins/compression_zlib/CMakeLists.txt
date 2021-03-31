cmake_minimum_required(VERSION 3.14)

project(blimp_plugin_compression_zlib)

option(BLIMP_PLUGIN_BUILD_COMPRESSION_ZLIB_TESTS "Determines whether to build tests for the zlib compression plugin" ON)

include(GenerateExportHeader)

add_library(compression_zlib SHARED)
target_sources(compression_zlib PRIVATE
    ${PROJECT_SOURCE_DIR}/compression_zlib.cpp
    ${PROJECT_SOURCE_DIR}/compression_zlib.hpp
)
generate_export_header(compression_zlib
    INCLUDE_GUARD_NAME BLIMP_INCLUDE_GUARD_PLUGIN_COMPRESSION_ZLIB_EXPORT_H
)
target_sources(compression_zlib PRIVATE ${PROJECT_BINARY_DIR}/compression_zlib_export.h)
target_include_directories(compression_zlib PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
target_link_libraries(compression_zlib PRIVATE blimp_plugin_sdk blimp_plugin_helper_cpp ZLIB::ZLIB)

if(BLIMP_PLUGIN_BUILD_COMPRESSION_ZLIB_TESTS)
    add_executable(compression_zlib_test ${PROJECT_SOURCE_DIR}/compression_zlib.t.cpp)
    target_include_directories(compression_zlib_test PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
    target_link_libraries(compression_zlib_test PUBLIC Catch2 blimp_plugin_sdk compression_zlib)
    add_test(NAME Plugin.Compression.zlib COMMAND compression_zlib_test)
    file(COPY ${ZLIB_DLL} ${ZLIBD_DLL} DESTINATION ${PROJECT_BINARY_DIR})
endif()
