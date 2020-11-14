cmake_minimum_required(VERSION 3.17)
project(_all)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Initialize the needed conan files. Allows us to link to the libraries.
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo_multi.cmake)
    include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo_multi.cmake)
else()
    include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
endif()
conan_basic_setup(TARGETS)

# Force high error level. This should simply be useful for preventing pulling in
# libraries with higher error levels later.
if (MSVC)
    add_compile_options(/W3 /WX)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
    add_compile_options(-W -Wall -Werror)
endif()

set(LIME_SOURCE_FILES
    inc/lime/cell.h
    inc/lime/column.h
    inc/lime/database.h
    inc/lime/execution.h
    inc/lime/placeholder.h
    inc/lime/record.h
    inc/lime/row.h
    inc/lime/table.h
    inc/lime/traits.h
    inc/lime/execution/all.h
    inc/lime/execution/context.h
    inc/lime/execution/each_row.h
    inc/lime/execution/function_caller.h
    inc/lime/execution/once.h
    inc/lime/execution/traits.h
    inc/lime/execution/types.h
    inc/lime/execution/utility.h
    src/database.cpp
)
add_library(lime STATIC ${LIME_SOURCE_FILES})
target_include_directories(lime PRIVATE inc)
target_link_libraries(lime ${CONAN_TARGETS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${LIME_SOURCE_FILES})