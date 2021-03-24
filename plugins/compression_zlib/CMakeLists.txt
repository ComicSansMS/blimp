cmake_minimum_required(VERSION 3.14)

project(blimp_plugin_compression_zlib)

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
target_link_libraries(compression_zlib PRIVATE blimp_plugin_sdk ZLIB::ZLIB)

