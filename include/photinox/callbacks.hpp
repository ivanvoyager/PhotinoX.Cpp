#pragma once

#include <photinox/event_args.hpp>

#include <exception>
#include <functional>

namespace photinox
{
    using InvokeStateCallback = void (*)(void* state);
    using ReleaseStateCallback = void (*)(void* state);

    using DispatcherCallback = std::function<void()>;
    using UnhandledExceptionHandler = std::function<void(std::exception_ptr exception)>;

    using StartupHandler = std::function<void()>;
    using ShutdownRequestedHandler = std::function<void(ShutdownRequestedEventArgs& args)>;
    using ExitHandler = std::function<void(ExitEventArgs& args)>;

    using WindowHandler = std::function<void()>;
    using ClosingHandler = std::function<void(ClosingEventArgs& args)>;
}