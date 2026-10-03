add_executable(PhotinoX.Cpp.CustomSchemes)

target_sources(
    PhotinoX.Cpp.CustomSchemes
    PRIVATE
        main.cpp
)

target_link_libraries(
    PhotinoX.Cpp.CustomSchemes
    PRIVATE
        PhotinoX::Cpp
)

photinox_copy_native_runtime(PhotinoX.Cpp.CustomSchemes)

add_custom_command(
    TARGET PhotinoX.Cpp.CustomSchemes
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/wwwroot"
        "$<TARGET_FILE_DIR:PhotinoX.Cpp.CustomSchemes>/wwwroot"
)