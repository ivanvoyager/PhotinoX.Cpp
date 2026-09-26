#pragma once

namespace photinox::native
{
    using InvokeStateCallback = void (*)(void* state);
    using ReleaseStateCallback = void (*)(void* state);
}