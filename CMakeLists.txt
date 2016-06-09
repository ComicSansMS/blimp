cmake_minimum_required(VERSION 3.2)

project(blimp)

add_subdirectory(external)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake")
if(WIN32)
    find_package(Qt5BaseDir)
    list(APPEND CMAKE_PREFIX_PATH ${QT5_BASE_DIR} ${WINSDK_LIB_DIR})
endif()
find_package(Qt5Widgets)

set(BLIMP_SOURCE_DIRECTORY ${PROJECT_SOURCE_DIR}/src)
set(BLIMP_INCLUDE_DIRECTORY ${PROJECT_SOURCE_DIR}/src)

set(BLIMP_SOURCE_FILES
    ${BLIMP_SOURCE_DIRECTORY}/main.cpp
)

set(BLIMP_HEADER_FILES
)

set(BLIMP_QT_SOURCE_FILES
    ${BLIMP_SOURCE_DIRECTORY}/ui/filesystem_model.cpp
    ${BLIMP_SOURCE_DIRECTORY}/ui/main_window.cpp
)

set(BLIMP_QT_MOC_HEADER_FILES
    ${BLIMP_INCLUDE_DIRECTORY}/ui/filesystem_model.hpp
    ${BLIMP_INCLUDE_DIRECTORY}/ui/main_window.hpp
)
qt5_wrap_cpp(BLIMP_QT_MOC_SOURCE_FILES ${BLIMP_QT_MOC_HEADER_FILES})

source_group(moc FILES
    ${BLIMP_QT_MOC_SOURCE_FILES}
)


add_executable(blimp ${BLIMP_SOURCE_FILES} ${BLIMP_HEADER_FILES} ${BLIMP_QT_SOURCE_FILES} ${BLIMP_QT_MOC_HEADER_FILES} ${BLIMP_QT_MOC_SOURCE_FILES})
target_include_directories(blimp PUBLIC ${BLIMP_INCLUDE_DIRECTORY})
target_link_libraries(blimp PUBLIC aws-cpp-sdk-glacier cryptopp-static gbBase GSL sqlite3 Qt5::Widgets)

if(WIN32)
    getQt5Dlls(Qt5::Widgets qt_DLLS)
    file(COPY ${qt_DLLS} ${QT5_ADDITIONAL_DLLS} DESTINATION ${PROJECT_BINARY_DIR})
endif()
