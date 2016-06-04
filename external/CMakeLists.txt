if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/aws-sdk-cpp/CMakeLists.txt)
    message(FATAL_ERROR "AWS SDK build file not found; Did you remember to fetch the submodules? (git submodule update --init)")
endif()
set(BUILD_ONLY glacier)
set(STATIC_LINKING "1")
add_subdirectory(aws-sdk-cpp)

if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/cryptopp/CMakeLists.txt)
    message(FATAL_ERROR "Cryptopp build file not found; Did you remember to fetch the submodules? (git submodule update --init)")
endif()
add_subdirectory(cryptopp)

if(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/GSL/CMakeLists.txt)
    message(FATAL_ERROR "GSL build file not found; Did you remember to fetch the submodules? (git submodule update --init)")
endif()
add_library(GSL INTERFACE)
target_include_directories(GSL INTERFACE ${CMAKE_CURRENT_LIST_DIR}/GSL/include)

add_subdirectory(sqlite3)
