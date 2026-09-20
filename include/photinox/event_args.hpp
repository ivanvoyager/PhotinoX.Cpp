#pragma once

#include <photinox/enums.hpp>

#include <any>
#include <string>

namespace photinox
{
    struct ShutdownRequestedEventArgs
    {
        ShutdownRequestReason reason;
        bool cancel = false;
    };

    struct ExitEventArgs
    {
        int applicationExitCode;
    };

    struct NotificationActivatedEventArgs
    {
        int notificationId;
        std::any state;
    };

    struct NotificationActionActivatedEventArgs
    {
        int notificationId;
        int actionIndex;
        std::any state;
    };

    struct NotificationInputActivatedEventArgs
    {
        int notificationId;
        std::string response;
        std::any state;
    };

    struct NotificationDismissedEventArgs
    {
        int notificationId;
        NotificationDismissalReason reason;
        std::any state;
    };

    struct NotificationFailedEventArgs
    {
        int notificationId;
        std::any state;
    };

    struct ClosingEventArgs
    {
        bool cancel = false;
    };
}