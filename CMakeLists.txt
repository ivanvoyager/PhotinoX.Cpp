cmake_minimum_required(VERSION 3.25)

project(
    PhotinoX.Cpp
    VERSION 5.2.2
    LANGUAGES CXX
)

include(cmake/PhotinoXNative.cmake)

include(FetchContent)

FetchContent_Declare(
    eventpp
    GIT_REPOSITORY https://github.com/wqking/eventpp.git
    GIT_TAG 1224dd6c9bd4577d686ac42334fc545997f5ece1
)

FetchContent_MakeAvailable(eventpp)

add_library(PhotinoX.Cpp STATIC)

add_library(PhotinoX::Cpp ALIAS PhotinoX.Cpp)

target_sources(
    PhotinoX.Cpp
    PRIVATE
        src/native/library.cpp
        src/native/library.hpp
        src/native/application.hpp
        src/native/notification.hpp
        src/native/window.hpp
        src/application.cpp
        src/dispatcher.cpp
        src/event_token.cpp
        src/window.cpp
        src/window_collection.cpp
    PUBLIC
        FILE_SET HEADERS
        BASE_DIRS include
        FILES
            include/photinox/application.hpp
            include/photinox/callbacks.hpp
            include/photinox/dispatcher.hpp
            include/photinox/enums.hpp
            include/photinox/event_args.hpp
            include/photinox/event_token.hpp
            include/photinox/photinox.hpp
            include/photinox/window.hpp
            include/photinox/window_collection.hpp
)

target_include_directories(
    PhotinoX.Cpp
    PRIVATE
        src
        "${eventpp_SOURCE_DIR}/include"
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