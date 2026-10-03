add_executable(PhotinoX.Cpp.NavigationPolicyDemo)

target_sources(
    PhotinoX.Cpp.NavigationPolicyDemo
    PRIVATE
        main.cpp
)

target_link_libraries(
    PhotinoX.Cpp.NavigationPolicyDemo
    PRIVATE
        PhotinoX::Cpp
)

if(WIN32)
    target_link_libraries(
        PhotinoX.Cpp.NavigationPolicyDemo
        PRIVATE
            Shell32
    )
endif()

photinox_copy_native_runtime(PhotinoX.Cpp.NavigationPolicyDemo)

add_custom_command(
    TARGET PhotinoX.Cpp.NavigationPolicyDemo
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/wwwroot"
        "$<TARGET_FILE_DIR:PhotinoX.Cpp.NavigationPolicyDemo>/wwwroot"
)