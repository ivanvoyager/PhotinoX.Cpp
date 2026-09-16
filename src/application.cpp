#include <photinox/application.hpp>

#include "native/library.hpp"
#include <eventpp/callbacklist.h>

#include <exception>
#include <string>
#include <utility>

namespace photinox
{
    class Application::Impl final
    {
    public:
        native::Library library;
        std::unique_ptr<Dispatcher> dispatcher;

        std::string name = "PhotinoX";
        std::string iconPath;
        std::string notificationRegistrationId = "PhotinoX";

        bool notificationsEnabled = true;

        eventpp::CallbackList<void()> startupHandlers;
        eventpp::CallbackList<void(ShutdownRequestedEventArgs&)> shutdownRequestedHandlers;
        eventpp::CallbackList<void(ExitEventArgs&)> exitHandlers;


        std::exception_ptr callbackException;

        native::ApplicationInitParams CreateInitParams() noexcept
        {
            native::ApplicationInitParams params{};

            params.size = sizeof(native::ApplicationInitParams);
            params.abiVersion = native::ApplicationInitParams::NativeAbiVersion;

            params.callbacks.startupHandler = StartupCallback;
            params.callbacks.shutdownRequestedHandler = ShutdownRequestedCallback;
            params.callbacks.exitHandler = ExitCallback;
            params.callbacks.windowCollectionChangedHandler = WindowCollectionChangedCallback;
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

            try
            {
                impl.startupHandlers();
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

            try
            {
                ShutdownRequestedEventArgs args
                {
                    .reason = reason
                };

                impl.shutdownRequestedHandlers(args);

                return args.cancel;
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

            try
            {
                ExitEventArgs args
                {
                    .applicationExitCode = exitCode
                };

                impl.exitHandlers(args);

                return args.applicationExitCode;
            }
            catch (...)
            {
                impl.callbackException = std::current_exception();
                return exitCode;
            }
        }

        static void WindowCollectionChangedCallback(NotifyCollectionChangedAction action, void* const* newItems, int newItemsCount, void* const* oldItems, int oldItemsCount, void* state) noexcept
        {
            auto& impl = *static_cast<Impl*>(state);

        }
    };

    Application::Application()
        : impl_(std::make_unique<Impl>())
    {
        impl_->dispatcher.reset(new Dispatcher(impl_->library));
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

    Application& Application::RegisterStartupHandler(StartupHandler handler)
    {
        impl_->startupHandlers.append(std::move(handler));
        return *this;
    }

    Application& Application::RegisterShutdownRequestedHandler(ShutdownRequestedHandler handler)
    {
        impl_->shutdownRequestedHandlers.append(std::move(handler));
        return *this;
    }

    Application& Application::RegisterExitHandler(ExitHandler handler)
    {
        impl_->exitHandlers.append(std::move(handler));
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

    Dispatcher& Application::GetDispatcher() noexcept
    {
        return *impl_->dispatcher;
    }

    const Dispatcher& Application::GetDispatcher() const noexcept
    {
        return *impl_->dispatcher;
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