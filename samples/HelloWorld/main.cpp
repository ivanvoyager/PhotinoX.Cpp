#include <photinox/photinox.hpp>

#include <iostream>

using namespace photinox;

int main()
{
    Application application;
    Window window(application);

    application.SetShutdownMode(ShutdownMode::OnExplicitShutdown);

    window
        .SetTitle("PhotinoX.Cpp HelloWorld")
        .LoadString(
            "<!DOCTYPE html>"
            "<html>"
            "<body>"
            "<h1>PhotinoX.Cpp</h1>"
            "</body>"
            "</html>")
        .RegisterCreatingHandler([]
        {
            std::cout << "Creating window" << '\n';
        })
        .RegisterCreatedHandler([&application]
        {
            std::cout << "Created window, count: " << application.Windows().size() << '\n';
        })
        .RegisterClosingHandler([](ClosingEventArgs& args)
        {
            std::cout << "Closing window" << '\n';
            args.cancel = false;
        })
        .RegisterClosedHandler([&application]
        {
            std::cout << "Closed window, count: " << application.Windows().size() << '\n';
#ifndef NDEBUG
            application.Shutdown(0, true);
#endif
        });

    application
        .SetName("PhotinoX.Cpp HelloWorld")
        .SetNotificationsEnabled(false)
        .RegisterStartupHandler([&application]
        {
            std::cout << "Startup handler" << '\n';
#ifdef NDEBUG
            application.Shutdown(0, true);
#endif
        })
        .RegisterExitHandler([](ExitEventArgs& args)
        {
            std::cout
                << "Exit handler: "
                << args.applicationExitCode
                << '\n';
        });

    return application.Run(&window);
}