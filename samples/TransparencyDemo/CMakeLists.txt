add_executable(PhotinoX.Cpp.TransparencyDemo)

target_sources(
    PhotinoX.Cpp.TransparencyDemo
    PRIVATE
        main.cpp
)

target_link_libraries(
    PhotinoX.Cpp.TransparencyDemo
    PRIVATE
        PhotinoX::Cpp
)

photinox_copy_native_runtime(PhotinoX.Cpp.TransparencyDemo)

add_custom_command(
    TARGET PhotinoX.Cpp.TransparencyDemo
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/wwwroot"
        "$<TARGET_FILE_DIR:PhotinoX.Cpp.TransparencyDemo>/wwwroot"
)