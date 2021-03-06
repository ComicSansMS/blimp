set(BUILD_SHARED OFF CACHE BOOL "")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "")

if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/aws-sdk-cpp/CMakeLists.txt)
    message(FATAL_ERROR "AWS SDK build file not found; Did you remember to fetch the submodules? (git submodule update --init)")
endif()
list(APPEND CMAKE_PREFIX_PATH ${BLIMP_BUILDBOX_DIRECTORY})
set(BUILD_DEPS OFF CACHE BOOL "" FORCE)
set(BUILD_ONLY "core;glacier" CACHE STRING "" FORCE)
set(STATIC_LINKING "1")
set(ENABLE_TESTING OFF CACHE BOOL "")
add_subdirectory(aws-sdk-cpp)

if(BLIMP_BUILD_TESTS)
    add_subdirectory(catch2)
endif()

find_package(Cryptopp REQUIRED)

if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/GhulbusBase/CMakeLists.txt)
    message(FATAL_ERROR "GhulbusBase build file not found; Did you remember to fetch the submodules? (git submodule update --init)")
endif()
set(GB_BUILD_TESTS OFF CACHE BOOL "")
add_subdirectory(GhulbusBase)

add_subdirectory(sqlite3)

set(SQLITE3_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/sqlite3)
set(SQLITE3_LIBRARY sqlite3)
set(ENABLE_TESTS OFF CACHE BOOL "")
set(HinnantDate_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/date)
add_subdirectory(sqlpp11-connector-sqlite3)
