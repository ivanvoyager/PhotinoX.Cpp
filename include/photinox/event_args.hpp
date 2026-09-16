#pragma once

#include <photinox/enums.hpp>

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
}