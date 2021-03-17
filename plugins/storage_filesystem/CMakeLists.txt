cmake_minimum_required(VERSION 3.14)

project(blimp_plugin_storage_filesystem)

include(GenerateExportHeader)

add_library(storage_filesystem SHARED)
target_sources(storage_filesystem PRIVATE
    ${PROJECT_SOURCE_DIR}/storage_filesystem.cpp
    ${PROJECT_SOURCE_DIR}/storage_filesystem.hpp
)
generate_export_header(storage_filesystem
    INCLUDE_GUARD_NAME BLIMP_INCLUDE_GUARD_PLUGIN_STORAGE_FILESYSTEM_EXPORT_H
)
target_sources(storage_filesystem PRIVATE ${PROJECT_BINARY_DIR}/storage_filesystem_export.h)
target_include_directories(storage_filesystem PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
target_link_libraries(storage_filesystem PRIVATE blimp_plugin_sdk)

