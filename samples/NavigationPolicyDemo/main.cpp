#include <photinox/photinox.hpp>

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>

#else

#include <spawn.h>

extern char** environ;

#endif

using namespace photinox;

namespace
{
#ifdef _WIN32
    std::wstring ToWideString(std::string_view value)
    {
        if (value.empty())
            return {};

        const int length = MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            value.data(),
            static_cast<int>(value.size()),
            nullptr,
            0);

        if (length <= 0)
            throw std::runtime_error("Failed to convert the URI to UTF-16.");

        std::wstring result(static_cast<std::size_t>(length), L'\0');

        if (MultiByteToWideChar(
            CP_UTF8,
            MB_ERR_INVALID_CHARS,
            value.data(),
            static_cast<int>(value.size()),
            result.data(),
            length) <= 0)
        {
            throw std::runtime_error("Failed to convert the URI to UTF-16.");
        }

        return result;
    }
#endif

    void OpenExternalBrowser(std::string_view uri)
    {
#ifdef _WIN32
        const std::wstring value = ToWideString(uri);
        const HINSTANCE result = ShellExecuteW(nullptr, L"open", value.c_str(), nullptr, nullptr, SW_SHOWNORMAL);

        if (reinterpret_cast<std::intptr_t>(result) <= 32)
            throw std::runtime_error("Failed to open the URI in the default browser.");
#else
        std::string value(uri);

#ifdef __APPLE__
        char executable[] = "open";
#else
        char executable[] = "xdg-open";
#endif

        char* arguments[]
        {
            executable,
            value.data(),
            nullptr
        };

        pid_t processId{};
        const int result = posix_spawnp(&processId, executable, nullptr, nullptr, arguments, environ);

        if (result != 0)
            throw std::runtime_error("Failed to open the URI in the default browser.");
#endif
    }

    bool IsExternalUri(std::string_view uri) noexcept
    {
        return uri.starts_with("http://") || uri.starts_with("https://");
    }

    void Log(const Window& window, std::string_view message)
    {
        std::cout
            << "-Client App: \"" << window.Title()
            << "\" " << message
            << '\n';
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
            .SetTitle("Navigation Policy Demo")
            .SetSize(960, 700)
            .Center()
            .SetStatusBarEnabled(false)
            .Load("wwwroot/navigation-policy.html")
            .RegisterNavigationStartingHandler(
                [&window](NavigationStartingEventArgs& args)
                {
                    Log(window, "NavigationStarting: " + args.uri);

                    if (IsExternalUri(args.uri))
                    {
                        args.cancel = true;
                        OpenExternalBrowser(args.uri);
                    }
                })
            .RegisterNewWindowRequestedHandler(
                [&window](const NewWindowRequestedEventArgs& args)
                {
                    Log(window, "NewWindowRequested: " + args.uri);
                    OpenExternalBrowser(args.uri);
                })
            .RegisterContentLoadedHandler(
                [&window](const ContentLoadedEventArgs& args)
                {
                    Log(window, "ContentLoaded: " + args.uri);
                })
            .RegisterWebMessageReceivedHandler(
                [&window](const WebMessageReceivedEventArgs& args)
                {
                    Log(window, "WebMessageReceived: " + args.message + "  Uri: " + args.uri);
                });

        return application.Run(&window);
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return 1;
    }
}