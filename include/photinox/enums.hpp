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

    enum class WindowEdge : int
    {
        Top,
        Bottom,
        Left,
        Right,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };
    static_assert(sizeof(WindowEdge) == sizeof(int));

    enum class HorizontalAlignment : int
    {
        Left = 0,
        Center = 1,
        Right = 2,
        Stretch = 3
    };
    static_assert(sizeof(HorizontalAlignment) == sizeof(int));

    enum class VerticalAlignment : int
    {
        Top = 0,
        Center = 1,
        Bottom = 2,
        Stretch = 3
    };
    static_assert(sizeof(VerticalAlignment) == sizeof(int));
}