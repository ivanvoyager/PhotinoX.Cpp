cmake_minimum_required(VERSION 3.25)

project(
    PhotinoX.Cpp
    VERSION 5.2.0
    LANGUAGES CXX
)

include(cmake/PhotinoXNative.cmake)

add_library(PhotinoX.Cpp STATIC)

add_library(PhotinoX::Cpp ALIAS PhotinoX.Cpp)

target_sources(
    PhotinoX.Cpp
    PRIVATE
        src/native/library.cpp
        src/native/library.hpp
        src/application.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES
            include/photinox/application.hpp
            include/photinox/error.hpp
            include/photinox/photinox.hpp
            include/photinox/result.hpp
            include/photinox/version.hpp
            include/photinox/window.hpp
)

target_include_directories(
    PhotinoX.Cpp
    PRIVATE
        src
)

target_compile_features(
    PhotinoX.Cpp
    PUBLIC
        cxx_std_20
)

set_target_properties(
    PhotinoX.Cpp
    PROPERTIES
        CXX_EXTENSIONS OFF
)

if(CMAKE_DL_LIBS)
    target_link_libraries(
        PhotinoX.Cpp
        PRIVATE
            ${CMAKE_DL_LIBS}
    )
endif()

if(MSVC)
    target_compile_options(
        PhotinoX.Cpp
        PRIVATE
            /W4
            /permissive-
    )
else()
    target_compile_options(
        PhotinoX.Cpp
        PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
endif()

option(PHOTINOX_BUILD_SAMPLES "Build PhotinoX.Cpp samples" ON)

if(PHOTINOX_BUILD_SAMPLES)
    add_subdirectory(samples/HelloWorld)
endif()