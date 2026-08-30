#pragma once

#include <photinox/enums.hpp>

#include <functional>

namespace photinox
{
    using InvokeStateCallback = void (*)(void* state);

    using StartupHandler = std::function<void()>;
    using ShutdownRequestedHandler = std::function<bool(ShutdownRequestReason reason)>;
    using ExitHandler = std::function<int(int exitCode)>;
}