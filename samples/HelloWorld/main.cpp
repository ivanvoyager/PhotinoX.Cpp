#include <photinox/photinox.hpp>

#include <iostream>

using namespace photinox;

int main()
{
    Application application;

    application
        .SetName("PhotinoX.Cpp HelloWorld")
        .SetNotificationsEnabled(false)
        .RegisterStartupHandler([&application]
        {
                std::cout << "First startup handler" << '\n';

                application.GetDispatcher().Invoke([]
                {
                    std::cout << "Dispatcher invoke" << '\n';
                });
        })
        .RegisterStartupHandler([&application]
        {
                std::cout << "Second startup handler" << '\n';
                application.Shutdown(0, true);
        })
        .RegisterExitHandler([](ExitEventArgs & args)
        {
            std::cout << "Exit handler: " << args.applicationExitCode << '\n';
        });

    std::cout << "Native: " << application.NativeVersion() << '\n';

    return application.Run();
}