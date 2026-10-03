add_executable(PhotinoX.Cpp.ChromelessDragResizeDemo)

target_sources(
    PhotinoX.Cpp.ChromelessDragResizeDemo
    PRIVATE
        main.cpp
)

target_link_libraries(
    PhotinoX.Cpp.ChromelessDragResizeDemo
    PRIVATE
        PhotinoX::Cpp
)

photinox_copy_native_runtime(PhotinoX.Cpp.ChromelessDragResizeDemo)

add_custom_command(
    TARGET PhotinoX.Cpp.ChromelessDragResizeDemo
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/wwwroot"
        "$<TARGET_FILE_DIR:PhotinoX.Cpp.ChromelessDragResizeDemo>/wwwroot"
)