#include <photinox/window.hpp>

#include <photinox/application.hpp>

#include "native/library.hpp"
#include "native/window.hpp"

#include "event_subscription.internal.hpp"

#include <eventpp/callbacklist.h>

#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

namespace photinox
{
    class Window::Impl final
    {
    public:
        explicit Impl(Application& application, Window* parent)
            : application(application),
            parent(parent)
        {
        }

        Application& application;
        Window* parent;

        std::string title = "PhotinoX";
        std::string startString;

        void* nativeInstance = nullptr;

        using WindowHandlerList = eventpp::CallbackList<void()>;
        using ClosingHandlerList = eventpp::CallbackList<void(ClosingEventArgs&)>;

        WindowHandlerList creatingHandlers;
        WindowHandlerList createdHandlers;
        ClosingHandlerList closingHandlers;
        WindowHandlerList closedHandlers;

        EventSubscriptionRegistry eventSubscriptions;

        std::unordered_map<std::uint64_t, WindowHandlerList::Handle> creatingHandlerSubscriptions;
        std::unordered_map<std::uint64_t, WindowHandlerList::Handle> createdHandlerSubscriptions;
        std::unordered_map<std::uint64_t, ClosingHandlerList::Handle> closingHandlerSubscriptions;
        std::unordered_map<std::uint64_t, WindowHandlerList::Handle> closedHandlerSubscriptions;

        bool isCreating = false;
        bool isClosed = false;
        bool forceClose = false;

        native::WindowInitParams CreateInitParams(Window* window) noexcept
        {
            native::WindowInitParams params{};

            params.size = sizeof(native::WindowInitParams);
            params.abiVersion = native::WindowInitParams::NativeAbiVersion;

            params.parentInstance =
                parent && parent->impl_->nativeInstance
                    ? parent->impl_->nativeInstance
                    : nullptr;

            params.callbacks.createdHandler = CreatedCallback;
            params.callbacks.closingHandler = ClosingCallback;
            params.callbacks.closedHandler = ClosedCallback;
            params.callbacks.callbackState = window;

            params.window.title = title.c_str();

            params.linuxChromeless.resizeBorderThickness = 8;

            params.geometry.maxWidth = std::numeric_limits<int>::max();
            params.geometry.maxHeight = std::numeric_limits<int>::max();
            params.geometry.windowState = WindowState::Normal;
            params.geometry.resizable = true;
            params.geometry.useOsDefaultLocation = true;
            params.geometry.useOsDefaultSize = true;

            params.browser.startString = startString.empty() ? nullptr : startString.c_str();

            params.browser.zoom = 100;
            params.browser.zoomEnabled = true;
            params.browser.contextMenuEnabled = true;
            params.browser.statusBarEnabled = true;
            params.browser.devToolsEnabled = true;
            params.browser.grantBrowserPermissions = true;
            params.browser.mediaAutoplayEnabled = true;
            params.browser.fileSystemAccessEnabled = true;
            params.browser.webSecurityEnabled = true;
            params.browser.javascriptClipboardAccessEnabled = true;
            params.browser.mediaStreamEnabled = true;
            params.browser.smoothScrollingEnabled = true;

            return params;
        }

        void ThrowIfClosed(std::string_view memberName) const
        {
            if (isClosed)
                throw std::logic_error(std::string(memberName) + " cannot be called after the window has been closed.");
        }

        void ThrowIfInitialized(std::string_view memberName) const
        {
            if (nativeInstance)
                throw std::logic_error(std::string(memberName) + " cannot be called after the window has been initialized.");
        }

        void ThrowIfNotInitialized(std::string_view memberName) const
        {
            if (!nativeInstance)
                throw std::logic_error(std::string(memberName) + " cannot be called before the window is initialized.");
        }

        void ThrowIfClosedOrNotInitialized(std::string_view memberName) const
        {
            ThrowIfClosed(memberName);
            ThrowIfNotInitialized(memberName);
        }

    private:

        static void CreatedCallback(void* instance, bool registered, void* state) noexcept
        {
            assert(instance);

            auto& window = *static_cast<Window*>(state);
            auto& impl = *window.impl_;

            assert(!impl.nativeInstance);

            impl.nativeInstance = instance;
            impl.application.OnWindowCreated(window, registered);

            try
            {
                impl.createdHandlers();
            }
            catch (...)
            {
                impl.application.OnUnhandledException(std::current_exception());
            }
        }

        static bool ClosingCallback(void* state) noexcept
        {
            auto& window = *static_cast<Window*>(state);
            auto& impl = *window.impl_;

            if (impl.forceClose)
                return false;

            try
            {
                ClosingEventArgs args;
                impl.closingHandlers(args);
                return args.cancel;
            }
            catch (...)
            {
                impl.application.OnUnhandledException(std::current_exception());
                return false;
            }
        }

        static void ClosedCallback(void* state) noexcept
        {
            auto& window = *static_cast<Window*>(state);
            auto& impl = *window.impl_;

            assert(impl.nativeInstance);

            impl.nativeInstance = nullptr;
            impl.isClosed = true;
            impl.forceClose = false;

            try
            {
                impl.closedHandlers();
            }
            catch (...)
            {
                impl.application.OnUnhandledException(std::current_exception());
            }

            impl.application.OnWindowClosed(window);
        }
    }; // class Window::Impl

    Window::Window(Application& application, Window* parent)
        : impl_(std::make_unique<Impl>(application, parent))
    {
    }

    Window::~Window()
    {
        assert(!impl_->nativeInstance);
        assert(impl_->isClosed || !impl_->application.IsRunning());

        if (impl_->nativeInstance)
            std::terminate();
    }

    // Properties

    // Title

    std::string_view Window::Title() const noexcept
    {
        return impl_->title;
    }

    Window& Window::SetTitle(std::string_view title)
    {
        impl_->ThrowIfClosed("SetTitle");

        if (!impl_->nativeInstance)
        {
            impl_->title = title;
            return *this;
        }

        impl_->title = impl_->application.GetDispatcher().Invoke([this, title = std::string(title)]
        {
            auto& library = impl_->application.NativeLibrary();

            library.WindowSetTitle(impl_->nativeInstance, title.c_str());
            return library.WindowGetTitle(impl_->nativeInstance);
        });

        return *this;
    }

    // Getters

    bool Window::IsInitialized() const noexcept
    {
        return impl_->nativeInstance != nullptr;
    }

    bool Window::IsClosed() const noexcept
    {
        return impl_->isClosed;
    }

    Application& Window::GetApplication() const noexcept
    {
        return impl_->application;
    }

    Window* Window::Parent() const noexcept
    {
        return impl_->parent;
    }

    // Methods

    void Window::InternalClose()
    {
        if (!impl_->nativeInstance || impl_->isClosed)
            return;

        impl_->forceClose = true;
        Close();
    }

    Window& Window::LoadString(std::string_view content)
    {
        impl_->ThrowIfClosed("LoadString");

        if (!impl_->nativeInstance)
        {
            impl_->startString = content;
            return *this;
        }

        impl_->application.GetDispatcher().Invoke([this, content = std::string(content)]
        {
            impl_->application.NativeLibrary().WindowNavigateToString(impl_->nativeInstance, content.c_str());
        });

        return *this;
    }

    void Window::Show()
    {
        impl_->ThrowIfClosed("Show");

        if (impl_->nativeInstance)
        {
            const bool shown = impl_->application.GetDispatcher().Invoke([this]
            {
                return impl_->application.NativeLibrary().WindowShow(impl_->nativeInstance);
            });

            if (!shown)
                throw std::runtime_error("Failed to show the window.");

            return;
        }

        if (impl_->isCreating)
            throw std::logic_error("The window is already being created.");

        impl_->application.GetDispatcher().VerifyAccessToCreateWindow();

        impl_->isCreating = true;

        try
        {
            impl_->creatingHandlers();
        }
        catch (...)
        {
            impl_->isCreating = false;
            throw;
        }

        impl_->isCreating = false;

        if (impl_->startString.empty())
        {
            throw std::invalid_argument("Initial browser content must be supplied with LoadString.");
        }

        auto params = impl_->CreateInitParams(this);

        void* nativeInstance =
            impl_->application.NativeLibrary().WindowCreate(&params);

        assert(impl_->nativeInstance == nativeInstance);

        if (!nativeInstance)
            throw std::runtime_error("Native window creation failed.");

        impl_->nativeInstance = nativeInstance;
    }

    void Window::Close()
    {
        impl_->ThrowIfClosedOrNotInitialized("Close");

        impl_->application.GetDispatcher().Invoke([this]
        {
            impl_->application.NativeLibrary().WindowClose(impl_->nativeInstance);
        });
    }

    // Event subscription methods

    // Creating Handlers

    Window& Window::RegisterCreatingHandler(WindowHandler handler)
    {
        impl_->ThrowIfClosed("RegisterCreatingHandler");
        RegisterEventHandler(impl_->creatingHandlers, std::move(handler));
        return *this;
    }

    EventToken Window::SubscribeCreatingHandler(WindowHandler handler)
    {
        impl_->ThrowIfClosed("SubscribeCreatingHandler");
        return impl_->eventSubscriptions.Subscribe(impl_->creatingHandlers, impl_->creatingHandlerSubscriptions, std::move(handler));
    }

    bool Window::UnsubscribeCreatingHandler(EventToken token)
    {
        return impl_->eventSubscriptions.Unsubscribe(impl_->creatingHandlers, impl_->creatingHandlerSubscriptions, token);
    }

    // Created Handlers

    Window& Window::RegisterCreatedHandler(WindowHandler handler)
    {
        impl_->ThrowIfClosed("RegisterCreatedHandler");
        RegisterEventHandler(impl_->createdHandlers, std::move(handler));
        return *this;
    }

    EventToken Window::SubscribeCreatedHandler(WindowHandler handler)
    {
        impl_->ThrowIfClosed("SubscribeCreatedHandler");
        return impl_->eventSubscriptions.Subscribe(impl_->createdHandlers, impl_->createdHandlerSubscriptions, std::move(handler));
    }

    bool Window::UnsubscribeCreatedHandler(EventToken token)
    {
        return impl_->eventSubscriptions.Unsubscribe(impl_->createdHandlers, impl_->createdHandlerSubscriptions, token);
    }

    // Closing Handlers

    Window& Window::RegisterClosingHandler(ClosingHandler handler)
    {
        impl_->ThrowIfClosed("RegisterClosingHandler");
        RegisterEventHandler(impl_->closingHandlers, std::move(handler));
        return *this;
    }

    EventToken Window::SubscribeClosingHandler(ClosingHandler handler)
    {
        impl_->ThrowIfClosed("SubscribeClosingHandler");
        return impl_->eventSubscriptions.Subscribe(impl_->closingHandlers, impl_->closingHandlerSubscriptions, std::move(handler));
    }

    bool Window::UnsubscribeClosingHandler(EventToken token)
    {
        return impl_->eventSubscriptions.Unsubscribe(impl_->closingHandlers, impl_->closingHandlerSubscriptions, token);
    }

    // Closed Handlers

    Window& Window::RegisterClosedHandler(WindowHandler handler)
    {
        impl_->ThrowIfClosed("RegisterClosedHandler");
        RegisterEventHandler(impl_->closedHandlers, std::move(handler));
        return *this;
    }

    EventToken Window::SubscribeClosedHandler(WindowHandler handler)
    {
        impl_->ThrowIfClosed("SubscribeClosedHandler");
        return impl_->eventSubscriptions.Subscribe(impl_->closedHandlers, impl_->closedHandlerSubscriptions, std::move(handler));
    }

    bool Window::UnsubscribeClosedHandler(EventToken token)
    {
        return impl_->eventSubscriptions.Unsubscribe(impl_->closedHandlers, impl_->closedHandlerSubscriptions, token);
    }
}