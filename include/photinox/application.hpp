#pragma once

#include <photinox/callbacks.hpp>
#include <photinox/dispatcher.hpp>
#include <photinox/event_token.hpp>
#include <photinox/window_collection.hpp>

#include <any>
#include <exception>
#include <memory>
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
        [[nodiscard]] WindowCollection& Windows() noexcept;
        [[nodiscard]] const WindowCollection& Windows() const noexcept;

        [[nodiscard]] int Run(Window* mainWindow = nullptr);

        void Shutdown(int exitCode = 0, bool force = false) const noexcept;

        [[nodiscard]] int ShowNotification(std::string_view title, std::string_view body, std::string_view iconPath = {}, std::any state = {});

        Application& RegisterStartupHandler(StartupHandler handler);
        [[nodiscard]] EventToken SubscribeStartupHandler(StartupHandler handler);
        bool UnsubscribeStartupHandler(EventToken token);

        Application& RegisterShutdownRequestedHandler(ShutdownRequestedHandler handler);
        [[nodiscard]] EventToken SubscribeShutdownRequestedHandler(ShutdownRequestedHandler handler);
        bool UnsubscribeShutdownRequestedHandler(EventToken token);

        Application& RegisterExitHandler(ExitHandler handler);
        [[nodiscard]] EventToken SubscribeExitHandler(ExitHandler handler);
        bool UnsubscribeExitHandler(EventToken token);

        Application& RegisterNotificationActivatedHandler(NotificationActivatedHandler handler);
        [[nodiscard]] EventToken SubscribeNotificationActivatedHandler(NotificationActivatedHandler handler);
        bool UnsubscribeNotificationActivatedHandler(EventToken token);

        Application& RegisterNotificationActionActivatedHandler(NotificationActionActivatedHandler handler);
        [[nodiscard]] EventToken SubscribeNotificationActionActivatedHandler(NotificationActionActivatedHandler handler);
        bool UnsubscribeNotificationActionActivatedHandler(EventToken token);

        Application& RegisterNotificationInputActivatedHandler(NotificationInputActivatedHandler handler);
        [[nodiscard]] EventToken SubscribeNotificationInputActivatedHandler(NotificationInputActivatedHandler handler);
        bool UnsubscribeNotificationInputActivatedHandler(EventToken token);

        Application& RegisterNotificationDismissedHandler(NotificationDismissedHandler handler);
        [[nodiscard]] EventToken SubscribeNotificationDismissedHandler(NotificationDismissedHandler handler);
        bool UnsubscribeNotificationDismissedHandler(EventToken token);

        Application& RegisterNotificationFailedHandler(NotificationFailedHandler handler);
        [[nodiscard]] EventToken SubscribeNotificationFailedHandler(NotificationFailedHandler handler);
        bool UnsubscribeNotificationFailedHandler(EventToken token);

    private:
        friend class Window;
        friend class WindowCollection;

        class Impl;
        std::unique_ptr<Impl> impl_;

        [[nodiscard]] native::Library& NativeLibrary() noexcept;

        void ThrowIfRunning(std::string_view memberName) const;

        void CloseWindows();
        void OnWindowCreated(Window& window, bool registered);
        void OnWindowClosed(Window& window);

        void OnUnhandledException(std::exception_ptr exception) const noexcept;
    };
}