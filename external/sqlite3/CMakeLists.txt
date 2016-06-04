cmake_minimum_required(VERSION 3.2)

project(sqlite3)

add_library(sqlite3 STATIC
    ${PROJECT_SOURCE_DIR}/sqlite3.c
    ${PROJECT_SOURCE_DIR}/sqlite3.h
    ${PROJECT_SOURCE_DIR}/sqlite3ext.h
)
target_include_directories(sqlite3 PUBLIC ${PROJECT_SOURCE_DIR})
