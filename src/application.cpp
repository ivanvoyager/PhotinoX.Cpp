#include <photinox/application.hpp>

#include "native/library.hpp"

#include <exception>
#include <string>
#include <utility>

namespace photinox
{
    class Application::Impl final
    {
    public:
        native::Library library;

        std::string name = "PhotinoX";
        std::string iconPath;
        std::string notificationRegistrationId = "PhotinoX";

        bool notificationsEnabled = true;

        StartupHandler startupHandler;
        ShutdownRequestedHandler shutdownRequestedHandler;
        ExitHandler exitHandler;

        std::exception_ptr callbackException;

        native::ApplicationInitParams CreateInitParams() noexcept
        {
            native::ApplicationInitParams params{};

            params.size = sizeof(native::ApplicationInitParams);
            params.abiVersion = native::ApplicationInitParams::NativeAbiVersion;

            params.callbacks.startupHandler = StartupCallback;
            params.callbacks.shutdownRequestedHandler = ShutdownRequestedCallback;
            params.callbacks.exitHandler = ExitCallback;
            params.callbacks.callbackState = this;

            params.options.applicationName = name.empty() ? nullptr : name.c_str();
            params.options.applicationIconPath = iconPath.empty() ? nullptr : iconPath.c_str();
            params.options.notificationRegistrationId =
                notificationRegistrationId.empty() ? nullptr : notificationRegistrationId.c_str();
            params.options.notificationsEnabled = notificationsEnabled;

            return params;
        }

    private:
        static void StartupCallback(void* state) noexcept
        {
            auto& impl = *static_cast<Impl*>(state);

            if (!impl.startupHandler)
                return;

            try
            {
                impl.startupHandler();
            }
            catch (...)
            {
                impl.callbackException = std::current_exception();
                impl.library.ApplicationShutdown(-1, true);
            }
        }

        static bool ShutdownRequestedCallback(ShutdownRequestReason reason, void* state) noexcept
        {
            auto& impl = *static_cast<Impl*>(state);

            if (!impl.shutdownRequestedHandler)
                return false;

            try
            {
                return impl.shutdownRequestedHandler(reason);
            }
            catch (...)
            {
                impl.callbackException = std::current_exception();
                return false;
            }
        }

        static int ExitCallback(int exitCode, void* state) noexcept
        {
            auto& impl = *static_cast<Impl*>(state);

            if (!impl.exitHandler)
                return exitCode;

            try
            {
                return impl.exitHandler(exitCode);
            }
            catch (...)
            {
                impl.callbackException = std::current_exception();
                return exitCode;
            }
        }
    };

    Application::Application()
        : impl_(std::make_unique<Impl>())
    {
    }

    Application::~Application() = default;

    Application::Application(Application&&) noexcept = default;

    Application& Application::operator=(Application&&) noexcept = default;

    Application& Application::SetName(std::string_view name)
    {
        impl_->name = name;
        return *this;
    }

    Application& Application::SetIconPath(std::string_view iconPath)
    {
        impl_->iconPath = iconPath;
        return *this;
    }

    Application& Application::SetNotificationRegistrationId(std::string_view registrationId)
    {
        impl_->notificationRegistrationId = registrationId;
        return *this;
    }

    Application& Application::SetNotificationsEnabled(bool enabled) noexcept
    {
        impl_->notificationsEnabled = enabled;
        return *this;
    }

    Application& Application::OnStartup(StartupHandler handler)
    {
        impl_->startupHandler = std::move(handler);
        return *this;
    }

    Application& Application::OnShutdownRequested(ShutdownRequestedHandler handler)
    {
        impl_->shutdownRequestedHandler = std::move(handler);
        return *this;
    }

    Application& Application::OnExit(ExitHandler handler)
    {
        impl_->exitHandler = std::move(handler);
        return *this;
    }

    std::string_view Application::NativeVersion() const noexcept
    {
        const char* version = impl_->library.GetVersion();
        return version ? std::string_view(version) : std::string_view();
    }

    bool Application::IsRunning() const noexcept
    {
        return impl_->library.ApplicationIsRunning();
    }

    bool Application::IsShuttingDown() const noexcept
    {
        return impl_->library.ApplicationIsShuttingDown();
    }

    bool Application::CheckAccess() const noexcept
    {
        return impl_->library.ApplicationCheckAccess();
    }

    bool Application::Invoke(InvokeStateCallback callback, void* state) const
    {
        return impl_->library.ApplicationInvoke(callback, state);
    }

    bool Application::BeginInvoke(InvokeStateCallback callback, void* state) const
    {
        return impl_->library.ApplicationBeginInvoke(callback, state);
    }

    int Application::Run()
    {
        impl_->callbackException = nullptr;

        auto params = impl_->CreateInitParams();
        const int exitCode = impl_->library.ApplicationRun(&params);

        if (impl_->callbackException)
            std::rethrow_exception(impl_->callbackException);

        return exitCode;
    }

    void Application::Shutdown(int exitCode, bool force) const noexcept
    {
        impl_->library.ApplicationShutdown(exitCode, force);
    }
}