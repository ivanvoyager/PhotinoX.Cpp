#include <photinox/photinox.hpp>

#include <exception>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

using namespace photinox;

namespace
{
    std::string GetFileUrl(const std::filesystem::path& path)
    {
        const std::u8string value = std::filesystem::absolute(path).generic_u8string();
        const std::string utf8(reinterpret_cast<const char*>(value.data()), value.size());

        if constexpr (Platform::IsWindows)
            return "file:///" + utf8;

        return "file://" + utf8;
    }

    std::optional<WindowEdge> ParseWindowEdge(std::string_view value) noexcept
    {
        if (value == "Top")
            return WindowEdge::Top;

        if (value == "Bottom")
            return WindowEdge::Bottom;

        if (value == "Left")
            return WindowEdge::Left;

        if (value == "Right")
            return WindowEdge::Right;

        if (value == "TopLeft")
            return WindowEdge::TopLeft;

        if (value == "TopRight")
            return WindowEdge::TopRight;

        if (value == "BottomLeft")
            return WindowEdge::BottomLeft;

        if (value == "BottomRight")
            return WindowEdge::BottomRight;

        return std::nullopt;
    }

    std::string_view ToString(WindowState state) noexcept
    {
        switch (state)
        {
            case WindowState::Normal:
                return "Normal";

            case WindowState::Minimized:
                return "Minimized";

            case WindowState::Maximized:
                return "Maximized";

            case WindowState::FullScreen:
                return "FullScreen";

            default:
                return "Unknown";
        }
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

        const std::string startUrl = GetFileUrl("wwwroot/chromeless.html");

        window
            .SetTitle("Chromeless Demo")
            .SetChromeless(true)
            .SetLinuxChromelessDragRegion(36, 110)
            .SetSize(520, 360)
            .SetMinSize(320, 200)
            .Center()
            .Load(startUrl)
            .RegisterWebMessageReceivedHandler(
                [&window](const WebMessageReceivedEventArgs& args)
                {
                    constexpr std::string_view resizePrefix = "beginresize-";

                    if (args.message == "begindrag")
                    {
                        window.BeginWindowDrag();
                        return;
                    }

                    if (args.message.starts_with(resizePrefix))
                    {
                        const auto edge = ParseWindowEdge(
                            std::string_view(args.message).substr(resizePrefix.size()));

                        if (edge)
                            window.BeginWindowResize(*edge);

                        return;
                    }

                    if (args.message == "minimize")
                    {
                        window.Minimize();
                        return;
                    }

                    if (args.message == "maximize")
                    {
                        if (window.GetWindowState() == WindowState::Maximized)
                            window.Restore();
                        else
                            window.Maximize();

                        return;
                    }

                    if (args.message == "close")
                    {
                        window.Close();
                        return;
                    }

                    std::cout << "Unknown message: " << args.message << '\n';
                })
            .RegisterLocationChangedHandler(
                [](const LocationChangedEventArgs& args)
                {
                    std::cout
                        << "Location changed. X: " << args.location.x
                        << ", Y: " << args.location.y
                        << '\n';
                })
            .RegisterSizeChangedHandler(
                [](const SizeChangedEventArgs& args)
                {
                    std::cout
                        << "Size changed. Width: " << args.size.width
                        << ", Height: " << args.size.height
                        << '\n';
                })
            .RegisterStateChangedHandler(
                [](const StateChangedEventArgs& args)
                {
                    std::cout
                        << "State changed. Old: " << ToString(args.oldState)
                        << ", New: " << ToString(args.newState)
                        << '\n';
                })
            .RegisterContentLoadedHandler(
                [](const ContentLoadedEventArgs& args)
                {
                    std::cout << "Content loaded. URI: " << args.uri << '\n';
                });

        return application.Run(&window);
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return 1;
    }
}