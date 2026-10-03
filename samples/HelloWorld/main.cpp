#include <photinox/photinox.hpp>

#include <algorithm>
#include <cassert>
#include <exception>
#include <iostream>

using namespace photinox;

int main()
{
    try
    {
        Application application;
        Window window(application);

        window
            .SetTitle("PhotinoX HelloWorld")
            .SetSize(900, 600)
            .Center()
            .LoadString(R"(
    <!DOCTYPE html>
    <html>
        <head>
            <meta charset="utf-8">
            <title>PhotinoX HelloWorld</title>
        </head>
        <body>
            <h1>Web messaging</h1>
            <button id="send-message">Send message to C++</button>

            <script>
                document
                    .getElementById("send-message")
                    .addEventListener("click", () =>
                    {
                        window.external.sendMessage("Hello from JavaScript");
                    });
            </script>
        </body>
    </html>
)")
        .RegisterCreatingHandler([]
        {
            std::cout << "Creating window\n";
        })
        .RegisterCreatedHandler([&application, &window]
        {
            std::cout << "Created window, count: " << application.Windows().Size() << '\n';

            const auto windows = application.Windows().Snapshot();

            assert(windows.size() == 1);
            assert(windows.front() == &window);
            assert(application.Windows().Contains(window));
            assert(!application.Windows().Empty());
        })
        .RegisterClosingHandler([](ClosingEventArgs& args)
        {
            std::cout << "Closing window\n";
            args.cancel = false;
        })
        .RegisterClosedHandler([&application]
        {
            std::cout << "Closed window, count: " << application.Windows().Size() << '\n';

            assert(application.Windows().Empty());
        })
        .RegisterWebMessageReceivedHandler([](const WebMessageReceivedEventArgs& args)
        {
            std::cout << "Message: " << args.message << '\n';
            std::cout << "Source: " << args.uri << '\n';
        });

        application
            .SetName("PhotinoX HelloWorld")
            .SetShutdownMode(ShutdownMode::OnMainWindowClose)
            .SetNotificationsEnabled(false)
            .RegisterStartupHandler([&application]
            {
                    std::cout << "Startup handler: " << application.Name() << '\n';
                    std::cout << "Notifications enabled: " << std::boolalpha << application.NotificationsEnabled() << '\n';

                    application.SetNotificationsEnabled(true);

                    std::cout << "Notifications enabled: " << application.NotificationsEnabled() << '\n';
            })
            .RegisterExitHandler([](ExitEventArgs& args)
            {
                    std::cout << "Exit handler: " << args.applicationExitCode << '\n';
            });

        return application.Run(&window);
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return 1;
    }
}