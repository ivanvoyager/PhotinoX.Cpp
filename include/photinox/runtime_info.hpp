#pragma once

#include <string>
#include <variant>

namespace photinox
{
    struct WindowsRuntimeInfo
    {
        std::string webView2RuntimeVersion;
    };

    struct LinuxRuntimeInfo
    {
        std::string glibcVersion;
        std::string gtkVersion;
        std::string webKitGtkApiTarget;
        std::string webKitGtkRuntimeVersion;
    };

    struct MacOSRuntimeInfo
    {
        std::string webKitVersion;
    };

    using PlatformRuntimeInfo = std::variant<
        WindowsRuntimeInfo,
        LinuxRuntimeInfo,
        MacOSRuntimeInfo>;

    struct RuntimeInfo
    {
        std::string nativeVersion;
        std::string webViewEngine;
        std::string webViewRuntimeVersion;
        PlatformRuntimeInfo platform;
    };
}