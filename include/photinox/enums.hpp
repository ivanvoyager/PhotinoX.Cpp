#pragma once

namespace photinox
{
    enum class ShutdownRequestReason : int
    {
        Unknown = 0,
        Application = 1,
        SessionLogoff = 2,
        SystemShutdown = 3
    };
    static_assert(sizeof(ShutdownRequestReason) == sizeof(int));

    enum class NotificationDismissalReason : int
    {
        Unknown = 0,
        UserCanceled = 1,
        ApplicationHidden = 2,
        TimedOut = 3
    };
    static_assert(sizeof(NotificationDismissalReason) == sizeof(int));
}