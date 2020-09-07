cmake_minimum_required(VERSION 3.17)
project(_all)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Initialize the needed conan files. Allows us to link to the libraries.
if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
else()
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
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

add_executable(
    lime_test
    src/main.cpp
)
target_link_libraries(lime_test PRIVATE ${CONAN_TARGETS})