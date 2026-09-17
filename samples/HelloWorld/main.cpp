#include <photinox/photinox.hpp>

#include <iostream>

using namespace photinox;

int main()
{
    Application application;
    Window window(application);

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
        .RegisterCreatedHandler([]
        {
            std::cout << "Created window" << '\n';
        })
        .RegisterClosedHandler([]
        {
            std::cout << "Closed window" << '\n';
        });

    application
        .SetName("PhotinoX.Cpp HelloWorld")
        .SetNotificationsEnabled(false)
        .RegisterStartupHandler([&window]
        {
            window.Show();
        })
        .RegisterExitHandler([](ExitEventArgs& args)
        {
            std::cout
                << "Exit handler: "
                << args.applicationExitCode
                << '\n';
        });

    return application.Run();
}