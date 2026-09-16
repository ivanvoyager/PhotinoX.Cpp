#pragma once

#include <photinox/event_args.hpp>

#include <functional>

namespace photinox
{
    using InvokeStateCallback = void (*)(void* state);

    using StartupHandler = std::function<void()>;
    using ShutdownRequestedHandler = std::function<void(ShutdownRequestedEventArgs& args)>;
    using ExitHandler = std::function<void(ExitEventArgs& args)>;
}