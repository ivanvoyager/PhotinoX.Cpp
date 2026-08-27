include(FetchContent)

set(
    PHOTINOX_NATIVE_VERSION
    "5.1.1"
    CACHE STRING
    "PhotinoX.Native package version"
)

string(TOLOWER "${PHOTINOX_NATIVE_VERSION}" photinox_native_version_lower)

if(WIN32)
    set(photinox_native_os "win")
    set(photinox_native_library "PhotinoX.Native.dll")
elseif(APPLE)
    set(photinox_native_os "osx")
    set(photinox_native_library "PhotinoX.Native.dylib")
elseif(UNIX)
    set(photinox_native_os "linux")
    set(photinox_native_library "PhotinoX.Native.so")
else()
    message(FATAL_ERROR "Unsupported operating system")
endif()

string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" photinox_native_processor)

if(photinox_native_processor MATCHES "^(amd64|x86_64)$")
    set(photinox_native_arch "x64")
elseif(photinox_native_processor MATCHES "^(aarch64|arm64)$")
    set(photinox_native_arch "arm64")
else()
    message(
        FATAL_ERROR
        "Unsupported processor architecture: ${CMAKE_SYSTEM_PROCESSOR}"
    )
endif()

set(
    PHOTINOX_NATIVE_RID
    "${photinox_native_os}-${photinox_native_arch}"
)

FetchContent_Declare(
    PhotinoXNative
    URL
        "https://api.nuget.org/v3-flatcontainer/photinox.native/${photinox_native_version_lower}/photinox.native.${photinox_native_version_lower}.nupkg"
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(PhotinoXNative)

set(
    PHOTINOX_NATIVE_DIRECTORY
    "${photinoxnative_SOURCE_DIR}/runtimes/${PHOTINOX_NATIVE_RID}/native"
)

set(
    PHOTINOX_NATIVE_LIBRARY
    "${PHOTINOX_NATIVE_DIRECTORY}/${photinox_native_library}"
)

if(NOT EXISTS "${PHOTINOX_NATIVE_LIBRARY}")
    message(
        FATAL_ERROR
        "PhotinoX.Native library was not found: ${PHOTINOX_NATIVE_LIBRARY}"
    )
endif()

function(photinox_copy_native_runtime target)
    add_custom_command(
        TARGET "${target}"
        POST_BUILD
        COMMAND
            "${CMAKE_COMMAND}" -E copy_directory
            "${PHOTINOX_NATIVE_DIRECTORY}"
            "$<TARGET_FILE_DIR:${target}>"
        VERBATIM
    )
endfunction()