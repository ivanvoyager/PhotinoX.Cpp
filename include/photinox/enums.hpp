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

    enum class NotifyCollectionChangedAction : int
    {
        Add = 0,
        Remove = 1,
        Replace = 2,
        Move = 3,
        Reset = 4
    };
    static_assert(sizeof(NotifyCollectionChangedAction) == sizeof(int));

    enum class WindowState : int
    {
        Normal,
        Minimized,
        Maximized,
        FullScreen
    };
    static_assert(sizeof(WindowState) == sizeof(int));

    enum class ShutdownMode : int
    {
        OnLastWindowClose,
        OnMainWindowClose,
        OnExplicitShutdown
    };
    static_assert(sizeof(ShutdownMode) == sizeof(int));
}