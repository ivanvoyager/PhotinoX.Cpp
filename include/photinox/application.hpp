#pragma once

#include <photinox/callbacks.hpp>
#include <photinox/dispatcher.hpp>

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

        Application(Application&&) noexcept;
        Application& operator=(Application&&) noexcept;

        Application& SetName(std::string_view name);
        Application& SetIconPath(std::string_view iconPath);
        Application& SetNotificationRegistrationId(std::string_view registrationId);
        Application& SetNotificationsEnabled(bool enabled) noexcept;

        Application& RegisterStartupHandler(StartupHandler handler);
        Application& RegisterShutdownRequestedHandler(ShutdownRequestedHandler handler);
        Application& RegisterExitHandler(ExitHandler handler);

        [[nodiscard]] std::string_view NativeVersion() const noexcept;

        [[nodiscard]] bool IsRunning() const noexcept;
        [[nodiscard]] bool IsShuttingDown() const noexcept;

        [[nodiscard]] Dispatcher& GetDispatcher() noexcept;
        [[nodiscard]] const Dispatcher& GetDispatcher() const noexcept;

        [[nodiscard]] int Run();

        void Shutdown(int exitCode = 0, bool force = false) const noexcept;

    private:
        friend class Window;

        [[nodiscard]] native::Library& NativeLibrary() noexcept;

        class Impl;
        std::unique_ptr<Impl> impl_;
    };
}