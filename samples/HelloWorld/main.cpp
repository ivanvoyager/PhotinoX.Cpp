#include <photinox/photinox.hpp>

#include <cassert>
#include <iostream>

using namespace photinox;

int main()
{   
    Application application;
    Window window(application);

    application.SetShutdownMode(ShutdownMode::OnMainWindowClose);

    const EventToken token = application.SubscribeStartupHandler([]
    {
    });

    assert(!application.UnsubscribeExitHandler(token));
    assert(application.UnsubscribeStartupHandler(token));

    const EventToken startupToken = application.SubscribeStartupHandler([]
    {
        std::cout << "Subscribed startup handler\n";
    });
    assert(startupToken);

    EventToken exitToken;

    exitToken = application.SubscribeExitHandler([&](ExitEventArgs&)
        {
            std::cout << "Subscribed exit handler\n";
            bool removed = application.UnsubscribeExitHandler(exitToken);
            assert(removed);
        });

    const EventToken noExitToken = application.SubscribeExitHandler([](ExitEventArgs&)
        {
            std::cout << "This handler must not run\n";
        });

    assert(application.UnsubscribeExitHandler(noExitToken));
    assert(!application.UnsubscribeExitHandler(noExitToken));

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
        });

    application
        .SetName("PhotinoX.Cpp HelloWorld")
        .SetNotificationsEnabled(false)
        .RegisterStartupHandler([&application]
        {
            std::cout << "Startup handler: " << application.Name() << '\n';
            std::cout << "Notifications enabled: " << application.NotificationsEnabled() << '\n';

            application.SetNotificationsEnabled(true);

            std::cout << "Notifications enabled: " << application.NotificationsEnabled() << '\n';

        #ifdef NDEBUG
            application.Shutdown(0, true);
        #endif
        })
        .RegisterExitHandler([](ExitEventArgs& args)
        {
            std::cout << "Exit handler: " << args.applicationExitCode  << '\n';
        });

    return application.Run(&window);
}