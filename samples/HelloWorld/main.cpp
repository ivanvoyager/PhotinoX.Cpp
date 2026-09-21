#include <photinox/photinox.hpp>

#include <cassert>
#include <iostream>

using namespace photinox;

int main()
{   
    Application application;
    Window window(application);

    application.SetShutdownMode(ShutdownMode::OnMainWindowClose);

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
        .RegisterCreatedHandler([&application, &window]
        {
            std::cout << "Created window, count: " << application.Windows().Size() << '\n';

            const auto windows = application.Windows().Snapshot();

            assert(windows.size() == 1);
            assert(windows.front() == &window);
            assert(application.Windows().Contains(window));
            assert(application.Windows().Size() == 1);
            assert(!application.Windows().Empty());
        })
        .RegisterClosingHandler([](ClosingEventArgs& args)
        {
            std::cout << "Closing window" << '\n';
            args.cancel = false;
        })
        .RegisterClosedHandler([&application]
        {
            std::cout << "Closed window, count: " << application.Windows().Size() << '\n';

            assert(application.Windows().Empty());
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

    application.RegisterNotificationActivatedHandler([](const NotificationActivatedEventArgs& args)
    {
        std::cout << "Notification activated: " << args.notificationId << '\n';

        if (const auto* state = std::any_cast<std::string>(&args.state))
            std::cout << "Notification state: " << *state << '\n';
    });

    application.RegisterNotificationActionActivatedHandler([](const NotificationActionActivatedEventArgs& args)
    {
        std::cout << "Notification action activated: " << args.notificationId << ", action: " << args.actionIndex << '\n';
    });

    application.RegisterNotificationInputActivatedHandler([](const NotificationInputActivatedEventArgs& args)
    {
        std::cout << "Notification input activated: " << args.notificationId << ", response: " << args.response << '\n';
    });

    application.RegisterNotificationDismissedHandler([](const NotificationDismissedEventArgs& args)
    {
        std::cout << "Notification dismissed: " << args.notificationId << " Reason: " << (int)args.reason << '\n';

        if (const auto* state = std::any_cast<std::string>(&args.state))
            std::cout << "Notification state: " << *state << '\n';
    });

    application.RegisterNotificationFailedHandler([](const NotificationFailedEventArgs& args)
    {
        std::cout << "Notification failed: " << args.notificationId << '\n';
    });

    application.RegisterStartupHandler([&application]
    {
        const int notificationId = application.ShowNotification(
            "PhotinoX",
            "Notification test",
            {},
            std::string("Test state"));

        std::cout << "ShowNotification result: " << notificationId << '\n';
    });

    application.Windows().RegisterChangedHandler(
    [](const WindowCollectionChangedEventArgs& args)
    {
        if (args.action == NotifyCollectionChangedAction::Add)
            std::cout << "Windows added: " << args.newItems.size() << '\n';

        if (args.action == NotifyCollectionChangedAction::Remove)
            std::cout << "Windows removed: " << args.oldItems.size() << '\n';
    });

    return application.Run(&window);
}