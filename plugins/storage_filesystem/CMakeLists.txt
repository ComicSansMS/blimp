cmake_minimum_required(VERSION 3.14)

project(blimp_plugin_storage_filesystem)

option(BLIMP_PLUGIN_BUILD_STORAGE_FILESYSTEM_TESTS "Determines whether to build tests for the filesystem storage plugin" ON)

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
target_link_libraries(storage_filesystem PRIVATE blimp_plugin_sdk blimp_plugin_helper_cpp)

if(BLIMP_PLUGIN_BUILD_STORAGE_FILESYSTEM_TESTS)
    add_executable(storage_filesystem_test ${PROJECT_SOURCE_DIR}/storage_filesystem.t.cpp)
    target_include_directories(storage_filesystem_test PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
    target_link_libraries(storage_filesystem_test PUBLIC Catch2 blimp_plugin_sdk storage_filesystem)
    add_test(NAME Plugin.Storage.Filesystem COMMAND storage_filesystem_test)
endif()
