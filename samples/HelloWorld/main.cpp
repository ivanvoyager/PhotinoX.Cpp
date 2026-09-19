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

            const bool scheduled = application.GetDispatcher().BeginInvoke([]
            {
                throw std::runtime_error("Dispatcher test exception.");
            });

            assert(scheduled);
        })
        .RegisterExitHandler([](ExitEventArgs& args)
        {
            std::cout << "Exit handler: " << args.applicationExitCode  << '\n';
        });


    const EventToken exceptionToken =
        application.GetDispatcher().SubscribeUnhandledExceptionHandler(
            [](std::exception_ptr exception)
            {
                try
                {
                    std::rethrow_exception(exception);
                }
                catch (const std::exception& ex)
                {
                    std::cout << "Unhandled dispatcher exception: " << ex.what() << '\n';
                }
            });

    return application.Run(&window);
}