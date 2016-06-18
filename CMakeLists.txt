cmake_minimum_required(VERSION 3.2)

project(blimp)

add_subdirectory(external)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
if(WIN32)
    find_package(Qt5BaseDir)
    list(APPEND CMAKE_PREFIX_PATH ${QT5_BASE_DIR} ${WINSDK_LIB_DIR})
endif()
find_package(Qt5Widgets 5.7 REQUIRED)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED filesystem)

set(BLIMP_SOURCE_DIRECTORY ${PROJECT_SOURCE_DIR}/src)
set(BLIMP_INCLUDE_DIRECTORY ${PROJECT_SOURCE_DIR}/src)

set(BLIMP_SOURCE_FILES
    ${BLIMP_SOURCE_DIRECTORY}/main.cpp
    ${BLIMP_SOURCE_DIRECTORY}/db/blimpdb.cpp
)

set(BLIMP_HEADER_FILES
    ${BLIMP_SOURCE_DIRECTORY}/exceptions.hpp
    ${BLIMP_SOURCE_DIRECTORY}/version.hpp
    ${BLIMP_SOURCE_DIRECTORY}/db/blimpdb.hpp
)

set(BLIMP_TABLE_HEADER_FILES
    ${BLIMP_SOURCE_DIRECTORY}/db/table/blimp_properties.hpp
    ${BLIMP_SOURCE_DIRECTORY}/db/table/sqlite_master.hpp
)
source_group("Table Headers" FILES ${BLIMP_TABLE_HEADER_FILES})

set(BLIMP_QT_SOURCE_FILES
    ${BLIMP_SOURCE_DIRECTORY}/ui/filesystem_model.cpp
    ${BLIMP_SOURCE_DIRECTORY}/ui/main_window.cpp
    ${BLIMP_SOURCE_DIRECTORY}/file_scanner.cpp
)

set(BLIMP_QT_MOC_HEADER_FILES
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
    ${BLIMP_QT_MOC_HEADER_FILES}
    ${BLIMP_QT_MOC_SOURCE_FILES}
)
target_include_directories(blimp PUBLIC ${BLIMP_INCLUDE_DIRECTORY})
target_link_libraries(blimp PUBLIC
    aws-cpp-sdk-glacier
    Boost::disable_autolinking
    Boost::filesystem
    cryptopp-static
    gbBase
    GSL
    sqlpp11-connector-sqlite3
    sqlite3
    Qt5::Widgets
)
target_include_directories(blimp PUBLIC external/date)
target_include_directories(blimp PUBLIC external/sqlpp11/include)
target_include_directories(blimp PUBLIC external/sqlpp11-connector-sqlite3/include)

if(WIN32)
    getQt5Dlls(Qt5::Widgets qt_DLLS)
    file(COPY ${qt_DLLS} ${QT5_ADDITIONAL_DLLS} DESTINATION ${PROJECT_BINARY_DIR})
    
    find_package(VLD QUIET)
    if(VLD_FOUND)
        message(STATUS "Found Visual Leak Detector.")
        target_compile_definitions(blimp PUBLIC BLIMP_HAS_VLD)
        target_include_directories(blimp PUBLIC ${VLD_INCLUDE_DIR})
        target_link_libraries(blimp PUBLIC $<$<CONFIG:Debug>:${VLD_LIBRARY}>)
        file(COPY ${VLD_RUNTIME_FILES} DESTINATION ${PROJECT_BINARY_DIR})
    endif()
endif()