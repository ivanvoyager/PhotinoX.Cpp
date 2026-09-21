#pragma once

#include <photinox/enums.hpp>

#include <any>
#include <span>
#include <string>

namespace photinox
{
    class Window;

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

    struct WindowCollectionChangedEventArgs
    {
        NotifyCollectionChangedAction action;
        std::span<Window* const> newItems;
        std::span<Window* const> oldItems;
    };

    struct ClosingEventArgs
    {
        bool cancel = false;
    };
}