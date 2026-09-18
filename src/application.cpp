#include <photinox/application.hpp>
#include <photinox/window.hpp>

#include "native/library.hpp"
#include <eventpp/callbacklist.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstdint>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace photinox
{
    namespace
    {
        bool IsValidShutdownMode(ShutdownMode shutdownMode) noexcept
        {
            switch (shutdownMode)
            {
                case ShutdownMode::OnLastWindowClose:
                case ShutdownMode::OnMainWindowClose:
                case ShutdownMode::OnExplicitShutdown:
                    return true;
                default:
                    return false;
            }
        }

        template<typename THandler>
        void ValidateHandler(const THandler& handler)
        {
            if (!handler)
                throw std::invalid_argument("handler");
        }
    }

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

        using StartupHandlerList = eventpp::CallbackList<void()>;
        using ShutdownRequestedHandlerList = eventpp::CallbackList<void(ShutdownRequestedEventArgs&)>;
        using ExitHandlerList = eventpp::CallbackList<void(ExitEventArgs&)>;

        StartupHandlerList startupHandlers;
        ShutdownRequestedHandlerList shutdownRequestedHandlers;
        ExitHandlerList exitHandlers;

        std::mutex eventSubscriptionsMutex;
        std::uint64_t nextEventToken = 1;

        std::unordered_map<std::uint64_t, StartupHandlerList::Handle> startupHandlerSubscriptions;
        std::unordered_map<std::uint64_t, ShutdownRequestedHandlerList::Handle> shutdownRequestedHandlerSubscriptions;
        std::unordered_map<std::uint64_t, ExitHandlerList::Handle> exitHandlerSubscriptions;

        std::exception_ptr callbackException;
        std::atomic_bool isRunning = false;
        std::atomic_bool notificationsEnabled = true;

        void SetCallbackException(std::exception_ptr exception) noexcept
        {
            if (!callbackException)
                callbackException = std::move(exception);
        }

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
            params.options.notificationsEnabled = notificationsEnabled.load(std::memory_order_acquire);

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
                impl.SetCallbackException(std::current_exception());
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
                impl.SetCallbackException(std::current_exception());
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
                impl.SetCallbackException(std::current_exception());
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
                impl.SetCallbackException(std::current_exception());
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

    native::Library& Application::NativeLibrary() noexcept
    {
        return impl_->library;
    }

    void Application::ThrowIfRunning(std::string_view memberName) const
    {
        if (IsRunning())
            throw std::logic_error(std::string(memberName) + " cannot be used after the application has started.");
    }

    EventToken Application::NextEventToken()
    {
        if (impl_->nextEventToken == 0)
            throw std::overflow_error("Application event token limit has been reached.");

        return EventToken(impl_->nextEventToken++);
    }

    std::string_view Application::Name() const noexcept
    {
        return impl_->name;
    }

    Application& Application::SetName(std::string_view name)
    {
        ThrowIfRunning("SetName");

        impl_->name = name;
        return *this;
    }

    std::string_view Application::IconPath() const noexcept
    {
        return impl_->iconPath;
    }

    Application& Application::SetIconPath(std::string_view iconPath)
    {
        ThrowIfRunning("SetIconPath");

        impl_->iconPath = iconPath;
        return *this;
    }

    bool Application::NotificationsEnabled() const
    {
        if (!IsRunning())
            return impl_->notificationsEnabled.load(std::memory_order_acquire);

        bool enabled = false;
        GetDispatcher().Invoke([this, &enabled]
        {
            enabled = impl_->library.ApplicationGetNotificationsEnabled();
        });
        return enabled;
    }

    Application& Application::SetNotificationsEnabled(bool enabled)
    {
        impl_->notificationsEnabled.store(enabled, std::memory_order_release);

        if (!IsRunning())
            return *this;

        GetDispatcher().Invoke([this, enabled]
        {
            impl_->library.ApplicationSetNotificationsEnabled(enabled);
        });
        return *this;
    }

    std::string_view Application::NotificationRegistrationId() const noexcept
    {
        return impl_->notificationRegistrationId;
    }

    Application& Application::SetNotificationRegistrationId(std::string_view registrationId)
    {
        ThrowIfRunning("SetNotificationRegistrationId");

        impl_->notificationRegistrationId = registrationId;
        return *this;
    }


    ShutdownMode Application::GetShutdownMode() const noexcept
    {
        return impl_->shutdownMode;
    }

    Application& Application::SetShutdownMode(ShutdownMode shutdownMode)
    {
        if (!IsValidShutdownMode(shutdownMode))
            throw std::invalid_argument("shutdownMode");

        if (IsShuttingDown())
            throw std::logic_error("Cannot change shutdown mode while the application is shutting down.");

        impl_->shutdownMode = shutdownMode;
        return *this;
    }

    Window* Application::MainWindow() const noexcept
    {
        return impl_->mainWindow;
    }

    std::span<Window* const> Application::Windows() const noexcept
    {
        return impl_->windows;
    }

    std::string_view Application::NativeVersion() const noexcept
    {
        const char* version = impl_->library.GetVersion();
        return version ? std::string_view(version) : std::string_view();
    }

    bool Application::IsRunning() const noexcept
    {
        return impl_->isRunning.load(std::memory_order_acquire);
    }

    bool Application::IsShuttingDown() const noexcept
    {
        return IsRunning() && impl_->library.ApplicationIsShuttingDown();
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
        if (impl_->isRunning.exchange(true, std::memory_order_acq_rel))
            throw std::logic_error("The application is already running.");

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
            impl_->isRunning.store(false, std::memory_order_release);

            return exitCode;
        }
        catch (...)
        {
            impl_->mainWindow = nullptr;
            impl_->isRunning.store(false, std::memory_order_release);
            throw;
        }
    }

    void Application::Shutdown(int exitCode, bool force) const noexcept
    {
        impl_->library.ApplicationShutdown(exitCode, force);
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

    Application& Application::RegisterStartupHandler(StartupHandler handler)
    {
        ValidateHandler(handler);

        impl_->startupHandlers.append(std::move(handler));
        return *this;
    }

    EventToken Application::SubscribeStartupHandler(StartupHandler handler)
    {
        ValidateHandler(handler);

        std::lock_guard lock(impl_->eventSubscriptionsMutex);

        const EventToken token = NextEventToken();
        auto handle = impl_->startupHandlers.append(std::move(handler));

        try
        {
            impl_->startupHandlerSubscriptions.emplace(token.value_, handle);
        }
        catch (...)
        {
            const bool removed = impl_->startupHandlers.remove(handle);
            assert(removed);
            throw;
        }

        return token;
    }

    bool Application::UnsubscribeStartupHandler(EventToken token)
    {
        if (!token)
            return false;

        std::lock_guard lock(impl_->eventSubscriptionsMutex);

        auto& subscriptions = impl_->startupHandlerSubscriptions;
        const auto iterator = subscriptions.find(token.value_);

        if (iterator == subscriptions.end())
            return false;

        const bool removed = impl_->startupHandlers.remove(iterator->second);
        assert(removed);

        subscriptions.erase(iterator);
        return removed;
    }

    Application& Application::RegisterShutdownRequestedHandler(ShutdownRequestedHandler handler)
    {
        ValidateHandler(handler);

        impl_->shutdownRequestedHandlers.append(std::move(handler));
        return *this;
    }

    EventToken Application::SubscribeShutdownRequestedHandler(ShutdownRequestedHandler handler)
    {
        ValidateHandler(handler);

        std::lock_guard lock(impl_->eventSubscriptionsMutex);

        const EventToken token = NextEventToken();
        auto handle = impl_->shutdownRequestedHandlers.append(std::move(handler));

        try
        {
            impl_->shutdownRequestedHandlerSubscriptions.emplace(token.value_, handle);
        }
        catch (...)
        {
            const bool removed = impl_->shutdownRequestedHandlers.remove(handle);
            assert(removed);
            throw;
        }

        return token;
    }

    bool Application::UnsubscribeShutdownRequestedHandler(EventToken token)
    {
        if (!token)
            return false;

        std::lock_guard lock(impl_->eventSubscriptionsMutex);

        auto& subscriptions = impl_->shutdownRequestedHandlerSubscriptions;
        const auto iterator = subscriptions.find(token.value_);

        if (iterator == subscriptions.end())
            return false;

        const bool removed = impl_->shutdownRequestedHandlers.remove(iterator->second);
        assert(removed);

        subscriptions.erase(iterator);
        return removed;
    }

    Application& Application::RegisterExitHandler(ExitHandler handler)
    {
        ValidateHandler(handler);

        impl_->exitHandlers.append(std::move(handler));
        return *this;
    }

    EventToken Application::SubscribeExitHandler(ExitHandler handler)
    {
        ValidateHandler(handler);

        std::lock_guard lock(impl_->eventSubscriptionsMutex);

        const EventToken token = NextEventToken();
        auto handle = impl_->exitHandlers.append(std::move(handler));

        try
        {
            impl_->exitHandlerSubscriptions.emplace(token.value_, handle);
        }
        catch (...)
        {
            const bool removed = impl_->exitHandlers.remove(handle);
            assert(removed);
            throw;
        }

        return token;
    }

    bool Application::UnsubscribeExitHandler(EventToken token)
    {
        if (!token)
            return false;

        std::lock_guard lock(impl_->eventSubscriptionsMutex);

        auto& subscriptions = impl_->exitHandlerSubscriptions;
        const auto iterator = subscriptions.find(token.value_);

        if (iterator == subscriptions.end())
            return false;

        const bool removed = impl_->exitHandlers.remove(iterator->second);
        assert(removed);

        subscriptions.erase(iterator);
        return removed;
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
}