add_executable(PhotinoX.Cpp.SplashScreen)

target_sources(
    PhotinoX.Cpp.SplashScreen
    PRIVATE
        main.cpp
)

target_link_libraries(
    PhotinoX.Cpp.SplashScreen
    PRIVATE
        PhotinoX::Cpp
)

photinox_copy_native_runtime(PhotinoX.Cpp.SplashScreen)

add_custom_command(
    TARGET PhotinoX.Cpp.SplashScreen
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/wwwroot"
        "$<TARGET_FILE_DIR:PhotinoX.Cpp.SplashScreen>/wwwroot"
)