cmake_minimum_required(VERSION 3.14)

project(blimp_plugin_encryption_aes)

option(BLIMP_PLUGIN_BUILD_ENCRYPTION_TESTS "Determines whether to build tests for the encryption plugin" ON)

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
target_link_libraries(encryption_aes PRIVATE blimp_plugin_sdk blimp_plugin_helper_cpp cryptopp-static)

if(BLIMP_PLUGIN_BUILD_ENCRYPTION_TESTS)
    add_executable(encryption_aes_test ${PROJECT_SOURCE_DIR}/encryption_aes.t.cpp)
    target_include_directories(encryption_aes_test PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})
    target_link_libraries(encryption_aes_test PUBLIC Catch2 blimp_plugin_sdk encryption_aes)
    add_test(NAME PluginEncryptionAES COMMAND encryption_aes_test)
endif()
