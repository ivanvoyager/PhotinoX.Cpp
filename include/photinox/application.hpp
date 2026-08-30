#pragma once

#include <photinox/callbacks.hpp>

#include <memory>
#include <string_view>

namespace photinox
{
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

        Application& OnStartup(StartupHandler handler);
        Application& OnShutdownRequested(ShutdownRequestedHandler handler);
        Application& OnExit(ExitHandler handler);

        [[nodiscard]] std::string_view NativeVersion() const noexcept;

        [[nodiscard]] bool IsRunning() const noexcept;
        [[nodiscard]] bool IsShuttingDown() const noexcept;
        [[nodiscard]] bool CheckAccess() const noexcept;

        [[nodiscard]] bool Invoke(InvokeStateCallback callback, void* state) const;
        [[nodiscard]] bool BeginInvoke(InvokeStateCallback callback, void* state) const;

        [[nodiscard]] int Run();

        void Shutdown(int exitCode = 0, bool force = false) const noexcept;

    private:
        class Impl;
        std::unique_ptr<Impl> impl_;
    };
}