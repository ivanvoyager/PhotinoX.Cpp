#include <photinox/application.hpp>
#include <photinox/window.hpp>

#include "native/library.hpp"
#include <eventpp/callbacklist.h>

#include <algorithm>
#include <cassert>
#include <exception>
#include <string>
#include <utility>
#include <vector>

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

        std::vector<Window*> windows;
        Window* mainWindow = nullptr;

        ShutdownMode shutdownMode = ShutdownMode::OnLastWindowClose;

        eventpp::CallbackList<void()> startupHandlers;
        eventpp::CallbackList<void(ShutdownRequestedEventArgs&)> shutdownRequestedHandlers;
        eventpp::CallbackList<void(ExitEventArgs&)> exitHandlers;


        std::exception_ptr callbackException;

        bool notificationsEnabled = true;

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

        static void WindowCollectionChangedCallback(NotifyCollectionChangedAction action, void* const* newItems, int newItemsCount,
                                                    void* const* oldItems, int oldItemsCount, void* state) noexcept
        {
            auto& impl = *static_cast<Impl*>(state);

            try
            {
                switch (action)
                {
                    case NotifyCollectionChangedAction::Add:
                        for (int i = 0; i < newItemsCount; ++i)
                        {
                            auto* window = static_cast<Window*>(newItems[i]);
                            assert(window);

                            if (window)
                                impl.windows.push_back(window);
                        }
                        break;

                    case NotifyCollectionChangedAction::Remove:
                        for (int i = 0; i < oldItemsCount; ++i)
                        {
                            auto* window = static_cast<Window*>(oldItems[i]);
                            assert(window);

                            const auto iterator =
                                std::find(impl.windows.begin(), impl.windows.end(), window);

                            assert(iterator != impl.windows.end());

                            if (iterator != impl.windows.end())
                                impl.windows.erase(iterator);
                        }
                        break;

                    default:
                        assert(false);
                        break;
                }
            }
            catch (...)
            {
                impl.callbackException = std::current_exception();
                impl.library.ApplicationShutdown(-1, true);
            }
        }
    };

    Application::Application()
        : impl_(std::make_unique<Impl>())
    {
        impl_->dispatcher.reset(new Dispatcher(impl_->library));
    }

    Application::~Application() = default;

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

    Application& Application::SetNotificationsEnabled(bool enabled) noexcept
    {
        impl_->notificationsEnabled = enabled;
        return *this;
    }

    Application& Application::SetNotificationRegistrationId(std::string_view registrationId)
    {
        impl_->notificationRegistrationId = registrationId;
        return *this;
    }

    Application& Application::SetShutdownMode(ShutdownMode shutdownMode)
    {
        if (IsShuttingDown())
            throw std::logic_error("Cannot change shutdown mode while the application is shutting down.");

        impl_->shutdownMode = shutdownMode;
        return *this;
    }

    ShutdownMode Application::GetShutdownMode() const noexcept
    {
        return impl_->shutdownMode;
    }

    Window* Application::MainWindow() const noexcept
    {
        return impl_->mainWindow;
    }

    std::span<Window* const> Application::Windows() const noexcept
    {
        return impl_->windows;
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

    int Application::Run(Window* mainWindow)
    {
        impl_->callbackException = nullptr;
        impl_->mainWindow = mainWindow;

        try
        {
            if (mainWindow)
                mainWindow->Show();

            auto params = impl_->CreateInitParams();
            const int exitCode = impl_->library.ApplicationRun(&params);

            assert(impl_->windows.empty());

            if (impl_->callbackException)
                std::rethrow_exception(impl_->callbackException);

            impl_->mainWindow = nullptr;
            return exitCode;
        }
        catch (...)
        {
            impl_->mainWindow = nullptr;
            throw;
        }
    }

    void Application::Shutdown(int exitCode, bool force) const noexcept
    {
        impl_->library.ApplicationShutdown(exitCode, force);
    }

    native::Library& Application::NativeLibrary() noexcept
    {
        return impl_->library;
    }

    void Application::OnWindowCreated(Window& window, bool registered)
    {
        assert(GetDispatcher().CheckAccess());

        if (registered)
        {
            assert(std::find(impl_->windows.begin(), impl_->windows.end(), &window) != impl_->windows.end());
            return;
        }

        assert(std::find(impl_->windows.begin(), impl_->windows.end(), &window) == impl_->windows.end());
        impl_->windows.push_back(&window);
    }

    void Application::OnWindowClosed(Window& window)
    {
        assert(GetDispatcher().CheckAccess());
        assert(std::find(impl_->windows.begin(), impl_->windows.end(), &window) == impl_->windows.end());

        const bool isMainWindow = impl_->mainWindow == &window;

        if (isMainWindow)
            impl_->mainWindow = nullptr;

        if (impl_->shutdownMode == ShutdownMode::OnExplicitShutdown)
            return;

        if (impl_->shutdownMode == ShutdownMode::OnMainWindowClose && isMainWindow)
        {
            Shutdown(0, true);
            return;
        }

        if (impl_->shutdownMode == ShutdownMode::OnLastWindowClose && impl_->windows.empty())
            Shutdown(0, true);
    }

    void Application::CloseWindows()
    {
        const auto windows = impl_->windows;

        for (auto iterator = windows.rbegin(); iterator != windows.rend(); ++iterator)
        {
            Window* window = *iterator;

            if (window)
                window->InternalClose();
        }
    }
}