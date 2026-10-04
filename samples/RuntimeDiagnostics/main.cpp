#include <photinox/photinox.hpp>

#include <nlohmann/json.hpp>

#include <exception>
#include <iostream>
#include <string_view>
#include <type_traits>
#include <variant>

using namespace photinox;
using Json = nlohmann::json;

namespace
{
    std::string_view GetPlatformName() noexcept
    {
        if constexpr (Platform::IsWindows)
            return "Windows";

        if constexpr (Platform::IsLinux)
            return "Linux";

        if constexpr (Platform::IsMacOS)
            return "MacOS";
    }

    std::string_view GetArchitecture() noexcept
    {
#if defined(_M_X64) || defined(__x86_64__)
        return "x64";
#elif defined(_M_ARM64) || defined(__aarch64__)
        return "arm64";
#elif defined(_M_IX86) || defined(__i386__)
        return "x86";
#elif defined(_M_ARM) || defined(__arm__)
        return "arm";
#else
        return "unknown";
#endif
    }

    Json CreateRuntimeInfoMessage(const RuntimeInfo& info)
    {
        Json result
        {
            { "Platform", GetPlatformName() },
            { "ProcessArchitecture", GetArchitecture() },
            { "LanguageRuntime", "C++20" },
            { "NativeVersion", info.nativeVersion },
            { "WebViewEngine", info.webViewEngine },
            { "WebViewRuntimeVersion", info.webViewRuntimeVersion }
        };

        std::visit(
            [&result](const auto& platformInfo)
            {
                using TPlatformInfo = std::decay_t<decltype(platformInfo)>;

                if constexpr (std::same_as<TPlatformInfo, WindowsRuntimeInfo>)
                {
                    result["Windows"] =
                    {
                        { "WebView2RuntimeVersion", platformInfo.webView2RuntimeVersion }
                    };
                }
                else if constexpr (std::same_as<TPlatformInfo, LinuxRuntimeInfo>)
                {
                    result["Linux"] =
                    {
                        { "GlibcVersion", platformInfo.glibcVersion },
                        { "GtkVersion", platformInfo.gtkVersion },
                        { "WebKitGtkApiTarget", platformInfo.webKitGtkApiTarget },
                        { "WebKitGtkRuntimeVersion", platformInfo.webKitGtkRuntimeVersion }
                    };
                }
                else if constexpr (std::same_as<TPlatformInfo, MacOSRuntimeInfo>)
                {
                    result["MacOS"] =
                    {
                        { "WebKitVersion", platformInfo.webKitVersion }
                    };
                }
            },
            info.platform);

        return result;
    }
}

int main()
{
    try
    {
        Application application;
        Window window(application);

        application
            .SetNotificationsEnabled(false)
            .SetShutdownMode(ShutdownMode::OnMainWindowClose);

        window
            .SetTitle("PhotinoX Runtime Diagnostics")
            .SetSize(800, 620)
            .Center()
            .Load("wwwroot/index.html")
            .RegisterWebMessageReceivedHandler(
                [&](const WebMessageReceivedEventArgs& args)
                {
                    if (args.message != "getRuntimeInfo")
                        return;

                    const RuntimeInfo runtimeInfo = application.GetRuntimeInfo();
                    window.SendWebMessage(CreateRuntimeInfoMessage(runtimeInfo).dump(2));
                });

        return application.Run(&window);
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return 1;
    }
}