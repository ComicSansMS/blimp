
find_path(CRYPTOPP_INCLUDE_DIR
    NAMES cryptlib.h
    HINTS ${CRYPTOPP_ROOT}
)

if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(TARGET_ARCHITECTURE Win32)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(TARGET_ARCHITECTURE x64)
else()
    message(FATAL_ERROR "Unrecognized pointer width")
endif()

find_library(CRYPTOPP_LIBRARY_DEBUG
    NAMES cryptlib
    HINTS ${CRYPTOPP_ROOT}
    PATH_SUFFIXES ${TARGET_ARCHITECTURE}/Output/Debug
)

find_library(CRYPTOPP_LIBRARY_RELEASE
    NAMES cryptlib
    HINTS ${CRYPTOPP_ROOT}
    PATH_SUFFIXES ${TARGET_ARCHITECTURE}/Output/Release
)

set(CRYPTOPP_LIBRARIES debug ${CRYPTOPP_LIBRARY_DEBUG} optimized ${CRYPTOPP_LIBRARY_RELEASE})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Cryptopp DEFAULT_MSG CRYPTOPP_INCLUDE_DIR CRYPTOPP_LIBRARY_DEBUG CRYPTOPP_LIBRARY_RELEASE)
mark_as_advanced(CRYPTOPP_INCLUDE_DIR CRYPTOPP_LIBRARY_DEBUG CRYPTOPP_LIBRARY_RELEASE)

add_library(cryptopp-static INTERFACE)
target_link_libraries(cryptopp-static INTERFACE ${CRYPTOPP_LIBRARIES})
target_include_directories(cryptopp-static INTERFACE ${CRYPTOPP_INCLUDE_DIR})
