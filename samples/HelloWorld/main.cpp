#include <photinox/photinox.hpp>

#include <iostream>

int main()
{
    photinox::Application application;

    application
        .SetName("PhotinoX.Cpp HelloWorld")
        .SetNotificationsEnabled(false)
        .OnStartup([&application]
        {
            std::cout << "Started" << '\n';
            application.Shutdown(0, true);
        })
        .OnExit([](int exitCode)
        {
            std::cout << "Exited: " << exitCode << '\n';
            return exitCode;
        });

    std::cout << "Native: " << application.NativeVersion() << '\n';

    return application.Run();
}