#pragma once

#include <type_traits>

namespace photinox::native
{
    struct WindowsRuntimeInfo
    {
        const char* webView2RuntimeVersion;
    };

    struct LinuxRuntimeInfo
    {
        const char* glibcVersion;
        const char* gtkVersion;
        const char* webKitGtkApiTarget;
        const char* webKitGtkRuntimeVersion;
    };

    struct MacOSRuntimeInfo
    {
        const char* webKitVersion;
    };

    struct RuntimeInfo
    {
        static constexpr int NativeAbiVersion = 2;

        int size;
        int abiVersion;

        const char* nativeVersion;
        const char* webViewEngine;
        const char* webViewRuntimeVersion;

        union
        {
            WindowsRuntimeInfo windows;
            LinuxRuntimeInfo linux;
            MacOSRuntimeInfo macOS;
        };
    };
    static_assert(std::is_standard_layout_v<RuntimeInfo>);
    static_assert(sizeof(RuntimeInfo) == 64);
}