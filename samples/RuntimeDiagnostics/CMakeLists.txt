include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)

FetchContent_MakeAvailable(nlohmann_json)

add_executable(PhotinoX.Cpp.RuntimeDiagnostics)

target_sources(
    PhotinoX.Cpp.RuntimeDiagnostics
    PRIVATE
        main.cpp
)

target_link_libraries(
    PhotinoX.Cpp.RuntimeDiagnostics
    PRIVATE
        PhotinoX::Cpp
        nlohmann_json::nlohmann_json
)

photinox_copy_native_runtime(PhotinoX.Cpp.RuntimeDiagnostics)

add_custom_command(
    TARGET PhotinoX.Cpp.RuntimeDiagnostics
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/wwwroot"
        "$<TARGET_FILE_DIR:PhotinoX.Cpp.RuntimeDiagnostics>/wwwroot"
)