cmake_minimum_required(VERSION 3.14)

project(blimp_plugin_encryption_aes)

include(GenerateExportHeader)

add_library(encryption_aes SHARED)
target_sources(encryption_aes PRIVATE
    ${PROJECT_SOURCE_DIR}/encryption_aes.cpp
    ${PROJECT_SOURCE_DIR}/encryption_aes.hpp
)
generate_export_header(encryption_aes
    INCLUDE_GUARD_NAME BLIMP_INCLUDE_GUARD_PLUGIN_ENCRYPTION_AES_EXPORT_H
)
target_sources(encryption_aes PRIVATE ${PROJECT_BINARY_DIR}/encryption_aes_export.h)
target_include_directories(encryption_aes PRIVATE ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
target_link_libraries(encryption_aes PRIVATE blimp_plugin_sdk cryptopp-static)

