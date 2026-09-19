#pragma once

#include <photinox/callbacks.hpp>
#include <photinox/dispatcher.hpp>
#include <photinox/event_token.hpp>

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

        [[nodiscard]] std::string_view Name() const noexcept;
        Application& SetName(std::string_view name);

        [[nodiscard]] std::string_view IconPath() const noexcept;
        Application& SetIconPath(std::string_view iconPath);

        [[nodiscard]] bool NotificationsEnabled() const;
        Application& SetNotificationsEnabled(bool enabled);

        [[nodiscard]] std::string_view NotificationRegistrationId() const noexcept;
        Application& SetNotificationRegistrationId(std::string_view registrationId);

        [[nodiscard]] ShutdownMode GetShutdownMode() const noexcept;
        Application& SetShutdownMode(ShutdownMode shutdownMode);

        [[nodiscard]] std::string_view NativeVersion() const noexcept;

        [[nodiscard]] bool IsRunning() const noexcept;
        [[nodiscard]] bool IsShuttingDown() const noexcept;

        [[nodiscard]] Dispatcher& GetDispatcher() noexcept;
        [[nodiscard]] const Dispatcher& GetDispatcher() const noexcept;

        [[nodiscard]] Window* MainWindow() const noexcept;
        [[nodiscard]] std::span<Window* const> Windows() const noexcept;

        [[nodiscard]] int Run(Window* mainWindow = nullptr);

        void Shutdown(int exitCode = 0, bool force = false) const noexcept;

        Application& RegisterStartupHandler(StartupHandler handler);
        [[nodiscard]] EventToken SubscribeStartupHandler(StartupHandler handler);
        bool UnsubscribeStartupHandler(EventToken token);

        Application& RegisterShutdownRequestedHandler(ShutdownRequestedHandler handler);
        [[nodiscard]] EventToken SubscribeShutdownRequestedHandler(ShutdownRequestedHandler handler);
        bool UnsubscribeShutdownRequestedHandler(EventToken token);

        Application& RegisterExitHandler(ExitHandler handler);
        [[nodiscard]] EventToken SubscribeExitHandler(ExitHandler handler);
        bool UnsubscribeExitHandler(EventToken token);

    private:
        friend class Window;

        class Impl;
        std::unique_ptr<Impl> impl_;

        [[nodiscard]] native::Library& NativeLibrary() noexcept;
        void ThrowIfRunning(std::string_view memberName) const;
        [[nodiscard]] EventToken NextEventToken();

        void CloseWindows();

        void OnWindowCreated(Window& window, bool registered);
        void OnWindowClosed(Window& window);
    };
}