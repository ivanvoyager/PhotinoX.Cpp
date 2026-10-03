include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)

FetchContent_MakeAvailable(nlohmann_json)

add_executable(PhotinoX.Cpp.NotificationDiagnostics)

target_sources(
    PhotinoX.Cpp.NotificationDiagnostics
    PRIVATE
        main.cpp
)

target_link_libraries(
    PhotinoX.Cpp.NotificationDiagnostics
    PRIVATE
        PhotinoX::Cpp
        nlohmann_json::nlohmann_json
)

photinox_copy_native_runtime(PhotinoX.Cpp.NotificationDiagnostics)

add_custom_command(
    TARGET PhotinoX.Cpp.NotificationDiagnostics
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/wwwroot"
        "$<TARGET_FILE_DIR:PhotinoX.Cpp.NotificationDiagnostics>/wwwroot"
    COMMAND
        ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:PhotinoX.Cpp.NotificationDiagnostics>/assets"
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/logo.png"
        "$<TARGET_FILE_DIR:PhotinoX.Cpp.NotificationDiagnostics>/assets/logo.png"
)