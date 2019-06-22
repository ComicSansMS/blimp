set(CATCH2_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR})
add_library(Catch2 STATIC ${CMAKE_CURRENT_LIST_DIR}/catch_main.cpp)
target_include_directories(Catch2 PUBLIC ${CATCH2_INCLUDE_DIR})
