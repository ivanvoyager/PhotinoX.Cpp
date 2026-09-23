#pragma once

#include <photinox/event_args.hpp>

#include <exception>
#include <functional>

namespace photinox
{
    //application
    using DispatcherCallback = std::function<void()>;
    using UnhandledExceptionHandler = std::function<void(std::exception_ptr exception)>;

    using StartupHandler = std::function<void()>;
    using ShutdownRequestedHandler = std::function<void(ShutdownRequestedEventArgs& args)>;
    using ExitHandler = std::function<void(ExitEventArgs& args)>;

    using WindowCollectionChangedHandler = std::function<void(const WindowCollectionChangedEventArgs& args)>;

    //notifications
    using NotificationActivatedHandler = std::function<void(const NotificationActivatedEventArgs& args)>;
    using NotificationActionActivatedHandler = std::function<void(const NotificationActionActivatedEventArgs& args)>;
    using NotificationInputActivatedHandler = std::function<void(const NotificationInputActivatedEventArgs& args)>;
    using NotificationDismissedHandler = std::function<void(const NotificationDismissedEventArgs& args)>;
    using NotificationFailedHandler = std::function<void(const NotificationFailedEventArgs& args)>;

    //window
    using WindowHandler = std::function<void()>;
    using ClosingHandler = std::function<void(ClosingEventArgs& args)>;

    using SizeChangedHandler = std::function<void(const SizeChangedEventArgs& args)>;
    using LocationChangedHandler = std::function<void(const LocationChangedEventArgs& args)>;
    using StateChangedHandler = std::function<void(const StateChangedEventArgs& args)>;
}