#include <photinox/window.hpp>

#include <photinox/application.hpp>

#include "native/library.hpp"
#include "native/window.hpp"

#include <eventpp/callbacklist.h>

#include <cassert>
#include <limits>
#include <stdexcept>
#include <string>
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

        bool isCreating = false;
        bool isClosed = false;

        eventpp::CallbackList<void()> creatingHandlers;
        eventpp::CallbackList<void()> createdHandlers;
        eventpp::CallbackList<void()> closedHandlers;

        native::WindowInitParams CreateInitParams() noexcept
        {
            native::WindowInitParams params{};

            params.size = sizeof(native::WindowInitParams);
            params.abiVersion = native::WindowInitParams::NativeAbiVersion;

            params.parentInstance =
                parent && parent->impl_->nativeInstance
                    ? parent->impl_->nativeInstance
                    : nullptr;

            params.callbacks.createdHandler = CreatedCallback;
            params.callbacks.closedHandler = ClosedCallback;
            params.callbacks.callbackState = this;

            params.window.title = title.c_str();

            params.linuxChromeless.resizeBorderThickness = 8;

            params.geometry.maxWidth = std::numeric_limits<int>::max();
            params.geometry.maxHeight = std::numeric_limits<int>::max();
            params.geometry.windowState = WindowState::Normal;
            params.geometry.resizable = true;
            params.geometry.useOsDefaultLocation = true;
            params.geometry.useOsDefaultSize = true;

            params.browser.startString =
                startString.empty() ? nullptr : startString.c_str();

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

    private:
        static void CreatedCallback(
            void* instance,
            bool registered,
            void* state) noexcept
        {
            auto& impl = *static_cast<Impl*>(state);

            assert(instance);
            assert(!impl.nativeInstance);

            impl.nativeInstance = instance;

            try
            {
                impl.createdHandlers();
            }
            catch (...)
            {
                impl.application.Shutdown(-1, true);
            }

            (void)registered;
        }

        static void ClosedCallback(void* state) noexcept
        {
            auto& impl = *static_cast<Impl*>(state);

            impl.nativeInstance = nullptr;
            impl.isClosed = true;

            try
            {
                impl.closedHandlers();
            }
            catch (...)
            {
                impl.application.Shutdown(-1, true);
            }
        }
    };

    Window::Window(Application& application, Window* parent)
        : impl_(std::make_unique<Impl>(application, parent))
    {
    }

    Window::~Window()
    {
        if (impl_->nativeInstance)
            Close();
    }

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

    Window& Window::SetTitle(std::string_view title)
    {
        if (impl_->isClosed)
            throw std::logic_error(
                "SetTitle cannot be called after the window has been closed.");

        if (impl_->nativeInstance)
        {
            throw std::logic_error(
                "SetTitle can currently only be called before the window is initialized.");
        }

        impl_->title = title;
        return *this;
    }

    Window& Window::LoadString(std::string_view content)
    {
        if (impl_->isClosed)
            throw std::logic_error(
                "LoadString cannot be called after the window has been closed.");

        if (impl_->nativeInstance)
        {
            throw std::logic_error(
                "LoadString can currently only be called before the window is initialized.");
        }

        impl_->startString = content;
        return *this;
    }

    Window& Window::RegisterCreatingHandler(WindowHandler handler)
    {
        if (!handler)
            throw std::invalid_argument("handler");

        if (impl_->isClosed)
            throw std::logic_error(
                "RegisterCreatingHandler cannot be called after the window has been closed.");

        impl_->creatingHandlers.append(std::move(handler));
        return *this;
    }

    Window& Window::RegisterCreatedHandler(WindowHandler handler)
    {
        if (!handler)
            throw std::invalid_argument("handler");

        if (impl_->isClosed)
            throw std::logic_error(
                "RegisterCreatedHandler cannot be called after the window has been closed.");

        impl_->createdHandlers.append(std::move(handler));
        return *this;
    }

    Window& Window::RegisterClosedHandler(WindowHandler handler)
    {
        if (!handler)
            throw std::invalid_argument("handler");

        if (impl_->isClosed)
            throw std::logic_error(
                "RegisterClosedHandler cannot be called after the window has been closed.");

        impl_->closedHandlers.append(std::move(handler));
        return *this;
    }

    void Window::Show()
    {
        if (impl_->isClosed)
        {
            throw std::logic_error(
                "Show cannot be called after the window has been closed.");
        }

        if (impl_->nativeInstance)
        {
            impl_->application.GetDispatcher().Invoke([this]
            {
                impl_->application.NativeLibrary().WindowShow(
                    impl_->nativeInstance);
            });

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
            throw std::invalid_argument(
                "Initial browser content must be supplied with LoadString.");
        }

        auto params = impl_->CreateInitParams();

        void* nativeInstance =
            impl_->application.NativeLibrary().WindowCreate(&params);

        assert(impl_->nativeInstance == nativeInstance);

        if (!nativeInstance)
            throw std::runtime_error("Native window creation failed.");

        impl_->nativeInstance = nativeInstance;
    }

    void Window::Close()
    {
        if (impl_->isClosed)
        {
            throw std::logic_error(
                "Close cannot be called after the window has been closed.");
        }

        if (!impl_->nativeInstance)
        {
            throw std::logic_error(
                "Close cannot be called before the window is initialized.");
        }

        impl_->application.GetDispatcher().Invoke([this]
        {
            impl_->application.NativeLibrary().WindowClose(
                impl_->nativeInstance);
        });
    }
}