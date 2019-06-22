cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0074 NEW)

project(blimp)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/permissive->)

if(MSVC)
    option(MULTIPROCESS_BUILD "Enables the /MP compile flag for Visual C++." ON)
    if(MULTIPROCESS_BUILD)
        set(MULTIPROCESS_LIMIT_BUILD_PROCESSES 0 CACHE STRING
            "If set to a positive number, this will be used as an argument to /MP.")
        if(NOT MULTIPROCESS_LIMIT_BUILD_PROCESSES)
            add_compile_options(/MP)
        else()
            add_compile_options(/MP${MULTIPROCESS_LIMIT_BUILD_PROCESSES})
        endif()
    endif()
endif()

option(BLIMP_BUILD_TESTS "Determines whether to build tests for blimp" ON)

add_subdirectory(external)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
find_package(Qt5BaseDir)
if(QT5_BASE_DIR)
    list(APPEND CMAKE_PREFIX_PATH ${QT5_BASE_DIR})
endif()
find_package(Qt5Widgets REQUIRED)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED filesystem)

set(BLIMP_SOURCE_DIRECTORY ${PROJECT_SOURCE_DIR}/src)
set(BLIMP_INCLUDE_DIRECTORY ${PROJECT_SOURCE_DIR}/src)

set(BLIMP_SOURCE_FILES
    ${BLIMP_SOURCE_DIRECTORY}/main.cpp
    ${BLIMP_SOURCE_DIRECTORY}/file_bundling.cpp
    ${BLIMP_SOURCE_DIRECTORY}/file_hash.cpp
    ${BLIMP_SOURCE_DIRECTORY}/file_processor.cpp
    ${BLIMP_SOURCE_DIRECTORY}/db/blimpdb.cpp
)

set(BLIMP_HEADER_FILES
    ${BLIMP_SOURCE_DIRECTORY}/exceptions.hpp
    ${BLIMP_SOURCE_DIRECTORY}/file_bundling.hpp
    ${BLIMP_SOURCE_DIRECTORY}/file_hash.hpp
    ${BLIMP_SOURCE_DIRECTORY}/file_info.hpp
    ${BLIMP_SOURCE_DIRECTORY}/file_processor.hpp
    ${BLIMP_SOURCE_DIRECTORY}/storage.hpp
    ${BLIMP_SOURCE_DIRECTORY}/storage_aws_glacier.hpp
    ${BLIMP_SOURCE_DIRECTORY}/storage_filesystem.hpp
    ${BLIMP_SOURCE_DIRECTORY}/version.hpp
    ${BLIMP_SOURCE_DIRECTORY}/db/blimpdb.hpp
)

set(BLIMP_TABLE_HEADER_FILES
    ${BLIMP_SOURCE_DIRECTORY}/db/table/table_layout.hpp

    ${BLIMP_SOURCE_DIRECTORY}/db/table/blimp_properties.hpp
    ${BLIMP_SOURCE_DIRECTORY}/db/table/file_contents.hpp
    ${BLIMP_SOURCE_DIRECTORY}/db/table/file_element.hpp
    ${BLIMP_SOURCE_DIRECTORY}/db/table/indexed_locations.hpp
    ${BLIMP_SOURCE_DIRECTORY}/db/table/snapshot.hpp
    ${BLIMP_SOURCE_DIRECTORY}/db/table/snapshot_contents.hpp
    ${BLIMP_SOURCE_DIRECTORY}/db/table/sqlite_master.hpp
    ${BLIMP_SOURCE_DIRECTORY}/db/table/user_selection.hpp
)
source_group("Table Headers" FILES ${BLIMP_TABLE_HEADER_FILES})

set(BLIMP_QT_SOURCE_FILES
    ${BLIMP_SOURCE_DIRECTORY}/ui/file_diff_model.cpp
    ${BLIMP_SOURCE_DIRECTORY}/ui/filesystem_model.cpp
    ${BLIMP_SOURCE_DIRECTORY}/ui/main_window.cpp
    ${BLIMP_SOURCE_DIRECTORY}/file_scanner.cpp
)

set(BLIMP_QT_HEADER_FILES
    ${BLIMP_SOURCE_DIRECTORY}/ui/filesize_to_string.hpp
)

set(BLIMP_QT_MOC_HEADER_FILES
    ${BLIMP_INCLUDE_DIRECTORY}/ui/file_diff_model.hpp
    ${BLIMP_INCLUDE_DIRECTORY}/ui/filesystem_model.hpp
    ${BLIMP_INCLUDE_DIRECTORY}/ui/main_window.hpp
    ${BLIMP_INCLUDE_DIRECTORY}/file_scanner.hpp
)
qt5_wrap_cpp(BLIMP_QT_MOC_SOURCE_FILES ${BLIMP_QT_MOC_HEADER_FILES})

source_group(moc FILES
    ${BLIMP_QT_MOC_SOURCE_FILES}
)

add_executable(blimp
    ${BLIMP_SOURCE_FILES}
    ${BLIMP_HEADER_FILES}
    ${BLIMP_TABLE_HEADER_FILES}
    ${BLIMP_QT_SOURCE_FILES}
    ${BLIMP_QT_HEADER_FILES}
    ${BLIMP_QT_MOC_HEADER_FILES}
    ${BLIMP_QT_MOC_SOURCE_FILES}
)

target_include_directories(blimp PUBLIC ${BLIMP_INCLUDE_DIRECTORY})
target_link_libraries(blimp PUBLIC
    aws-cpp-sdk-core
    aws-cpp-sdk-glacier
    Boost::disable_autolinking
    Boost::filesystem
    cryptopp-static
    gbBase
    sqlpp11-connector-sqlite3
    sqlite3
    Qt5::Widgets
)
target_include_directories(blimp PUBLIC external/date)
target_include_directories(blimp PUBLIC external/sqlpp11/include)
target_include_directories(blimp PUBLIC external/sqlpp11-connector-sqlite3/include)
target_compile_definitions(blimp PUBLIC $<$<STREQUAL:$<CXX_COMPILER_ID>,MSVC>:_CRT_SECURE_NO_WARNINGS>)
target_compile_options(blimp PUBLIC $<$<AND:$<STREQUAL:$<CXX_COMPILER_ID>,MSVC>,$<NOT:$<CONFIG:Debug>>>:/Zo>)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT blimp)

add_executable(aws_tester src/aws_prototype.cpp)
target_link_libraries(aws_tester PUBLIC aws-cpp-sdk-core aws-cpp-sdk-glacier cryptopp-static)

if(WIN32)
    getQt5Dlls(Qt5::Widgets qt_DLLS)
    file(COPY ${qt_DLLS} ${QT5_ADDITIONAL_DLLS} DESTINATION ${PROJECT_BINARY_DIR})
endif()

if(BLIMP_BUILD_TESTS)
    enable_testing()

    add_executable(test_blimp_ui
        ${PROJECT_SOURCE_DIR}/test/ui/filesize_to_string.t.cpp
    )
    target_include_directories(test_blimp_ui PUBLIC ${BLIMP_INCLUDE_DIRECTORY})
    target_link_libraries(test_blimp_ui PUBLIC Catch2 Qt5::Widgets)
    add_test(NAME BlimpUI COMMAND test_blimp_ui)
endif()
