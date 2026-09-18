#pragma once

#include <photinox/callbacks.hpp>
#include <photinox/dispatcher.hpp>

#include <memory>
#include <span>
#include <string_view>

namespace photinox::native
{
    class Library;
}

namespace photinox
{
    class Window;

    class Application final
    {
    public:
        Application();
        ~Application();

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        Application(Application&&) = delete;
        Application& operator=(Application&&) = delete;

        Application& SetName(std::string_view name);
        Application& SetIconPath(std::string_view iconPath);
        Application& SetNotificationsEnabled(bool enabled) noexcept;
        Application& SetNotificationRegistrationId(std::string_view registrationId);
        Application& SetShutdownMode(ShutdownMode shutdownMode);

        Application& RegisterStartupHandler(StartupHandler handler);
        Application& RegisterShutdownRequestedHandler(ShutdownRequestedHandler handler);
        Application& RegisterExitHandler(ExitHandler handler);

        [[nodiscard]] std::string_view NativeVersion() const noexcept;

        [[nodiscard]] bool IsRunning() const noexcept;
        [[nodiscard]] bool IsShuttingDown() const noexcept;

        [[nodiscard]] Dispatcher& GetDispatcher() noexcept;
        [[nodiscard]] const Dispatcher& GetDispatcher() const noexcept;

        [[nodiscard]] ShutdownMode GetShutdownMode() const noexcept;
        [[nodiscard]] Window* MainWindow() const noexcept;
        [[nodiscard]] std::span<Window* const> Windows() const noexcept;

        [[nodiscard]] int Run(Window* mainWindow = nullptr);

        void Shutdown(int exitCode = 0, bool force = false) const noexcept;

    private:
        friend class Window;

        class Impl;
        std::unique_ptr<Impl> impl_;

        [[nodiscard]] native::Library& NativeLibrary() noexcept;

        void OnWindowCreated(Window& window, bool registered);
        void OnWindowClosed(Window& window);

        void CloseWindows();
    };
}