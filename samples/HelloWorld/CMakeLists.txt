add_executable(PhotinoX.Cpp.HelloWorld)

target_sources(
    PhotinoX.Cpp.HelloWorld
    PRIVATE
        main.cpp
)

target_link_libraries(
    PhotinoX.Cpp.HelloWorld
    PRIVATE
        PhotinoX::Cpp
)

photinox_copy_native_runtime(PhotinoX.Cpp.HelloWorld)