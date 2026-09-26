#include <photinox/window.hpp>

#include <photinox/application.hpp>
#include <photinox/dispatcher.hpp>

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
    namespace
    {
        bool IsValidWindowState(WindowState state) noexcept
        {
            switch (state)
            {
                case WindowState::Normal:
                case WindowState::Minimized:
                case WindowState::Maximized:
                case WindowState::FullScreen:
                    return true;
                default:
                    return false;
            }
        }
    } // namespace

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
        std::string iconFile;
        std::string startString;
        std::string startUrl;

        Point location;
        Size size;
        Size minSize;
        Size maxSize
        {
            .width = std::numeric_limits<int>::max(),
            .height = std::numeric_limits<int>::max()
        };

        WindowState windowState = WindowState::Normal;

        bool resizable = true;
        bool topmost = false;
        bool centerOnInitialize = false;
        bool useOsDefaultLocation = true;
        bool useOsDefaultSize = true;
        bool useNativeWindowOwner = false;
        bool chromeless = false;
        bool transparent = false;

        void* nativeInstance = nullptr;

        int zoom = 100;
        bool contextMenuEnabled = true;
        bool zoomEnabled = true;
        bool statusBarEnabled = true;
        bool devToolsEnabled = true;

        bool grantBrowserPermissions = true;
        bool mediaAutoplayEnabled = true;
        bool fileSystemAccessEnabled = true;
        bool webSecurityEnabled = true;
        bool javascriptClipboardAccessEnabled = true;
        bool mediaStreamEnabled = true;
        bool smoothScrollingEnabled = true;
        bool ignoreCertificateErrorsEnabled = false;

        using WindowHandlerList = eventpp::CallbackList<void()>;
        using ClosingHandlerList = eventpp::CallbackList<void(ClosingEventArgs&)>;

        using SizeChangedHandlerList = eventpp::CallbackList<void(const SizeChangedEventArgs&)>;
        using LocationChangedHandlerList = eventpp::CallbackList<void(const LocationChangedEventArgs&)>;
        using StateChangedHandlerList = eventpp::CallbackList<void(const StateChangedEventArgs&)>;

        using WebMessageReceivedHandlerList = eventpp::CallbackList<void(const WebMessageReceivedEventArgs&)>;
        using NavigationStartingHandlerList = eventpp::CallbackList<void(NavigationStartingEventArgs&)>;
        using NewWindowRequestedHandlerList = eventpp::CallbackList<void(const NewWindowRequestedEventArgs&)>;
        using ContentLoadingHandlerList = eventpp::CallbackList<void(const ContentLoadingEventArgs&)>;
        using ContentLoadedHandlerList = eventpp::CallbackList<void(const ContentLoadedEventArgs&)>;

        WindowHandlerList creatingHandlers;
        WindowHandlerList createdHandlers;
        ClosingHandlerList closingHandlers;
        WindowHandlerList closedHandlers;
        WindowHandlerList activatedHandlers;
        WindowHandlerList deactivatedHandlers;
        SizeChangedHandlerList sizeChangedHandlers;
        LocationChangedHandlerList locationChangedHandlers;
        WindowHandlerList maximizedHandlers;
        WindowHandlerList restoredHandlers;
        WindowHandlerList minimizedHandlers;
        WindowHandlerList fullScreenEnteredHandlers;
        WindowHandlerList fullScreenExitedHandlers;
        StateChangedHandlerList stateChangedHandlers;
        WebMessageReceivedHandlerList webMessageReceivedHandlers;
        NavigationStartingHandlerList navigationStartingHandlers;
        NewWindowRequestedHandlerList newWindowRequestedHandlers;
        ContentLoadingHandlerList contentLoadingHandlers;
        ContentLoadedHandlerList contentLoadedHandlers;
        ContentLoadedHandlerList initialContentLoadedHandlers;

        EventSubscriptionRegistry eventSubscriptions;

        std::unordered_map<std::uint64_t, WindowHandlerList::Handle> creatingHandlerSubscriptions;
        std::unordered_map<std::uint64_t, WindowHandlerList::Handle> createdHandlerSubscriptions;
        std::unordered_map<std::uint64_t, ClosingHandlerList::Handle> closingHandlerSubscriptions;
        std::unordered_map<std::uint64_t, WindowHandlerList::Handle> closedHandlerSubscriptions;
        std::unordered_map<std::uint64_t, WindowHandlerList::Handle> activatedHandlerSubscriptions;
        std::unordered_map<std::uint64_t, WindowHandlerList::Handle> deactivatedHandlerSubscriptions;
        std::unordered_map<std::uint64_t, SizeChangedHandlerList::Handle> sizeChangedHandlerSubscriptions;
        std::unordered_map<std::uint64_t, LocationChangedHandlerList::Handle> locationChangedHandlerSubscriptions;
        std::unordered_map<std::uint64_t, WindowHandlerList::Handle> maximizedHandlerSubscriptions;
        std::unordered_map<std::uint64_t, WindowHandlerList::Handle> restoredHandlerSubscriptions;
        std::unordered_map<std::uint64_t, WindowHandlerList::Handle> minimizedHandlerSubscriptions;
        std::unordered_map<std::uint64_t, WindowHandlerList::Handle> fullScreenEnteredHandlerSubscriptions;
        std::unordered_map<std::uint64_t, WindowHandlerList::Handle> fullScreenExitedHandlerSubscriptions;
        std::unordered_map<std::uint64_t, StateChangedHandlerList::Handle> stateChangedHandlerSubscriptions;
        std::unordered_map<std::uint64_t, WebMessageReceivedHandlerList::Handle> webMessageReceivedHandlerSubscriptions;
        std::unordered_map<std::uint64_t, NavigationStartingHandlerList::Handle> navigationStartingHandlerSubscriptions;
        std::unordered_map<std::uint64_t, NewWindowRequestedHandlerList::Handle> newWindowRequestedHandlerSubscriptions;
        std::unordered_map<std::uint64_t, ContentLoadingHandlerList::Handle> contentLoadingHandlerSubscriptions;
        std::unordered_map<std::uint64_t, ContentLoadedHandlerList::Handle> contentLoadedHandlerSubscriptions;
        std::unordered_map<std::uint64_t, ContentLoadedHandlerList::Handle> initialContentLoadedHandlerSubscriptions;

        bool isCreating = false;
        bool isClosed = false;
        bool forceClose = false;
        bool initialContentLoadedRaised = false;

        native::WindowInitParams CreateInitParams(Window* window, bool showOnInitialize) noexcept
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
            params.callbacks.focusInHandler = FocusInCallback;
            params.callbacks.focusOutHandler = FocusOutCallback;
            params.callbacks.resizedHandler = ResizedCallback;
            params.callbacks.movedHandler = MovedCallback;
            params.callbacks.maximizedHandler = MaximizedCallback;
            params.callbacks.restoredHandler = RestoredCallback;
            params.callbacks.minimizedHandler = MinimizedCallback;
            params.callbacks.fullScreenChangedHandler = FullScreenChangedCallback;
            params.callbacks.stateChangedHandler = StateChangedCallback;
            params.callbacks.webMessageReceivedHandler = WebMessageReceivedCallback;

            params.callbacks.navigationStartingHandler = NavigationStartingCallback;
            params.callbacks.newWindowRequestedHandler = NewWindowRequestedCallback;
            params.callbacks.contentLoadingHandler = ContentLoadingCallback;
            params.callbacks.contentLoadedHandler = ContentLoadedCallback;

            params.callbacks.callbackState = window;

            params.window.title = title.c_str();
            params.window.iconFile = iconFile.empty() ? nullptr : iconFile.c_str();
            params.window.chromeless = chromeless;
            params.window.transparent = transparent;
            params.window.useNativeWindowOwner = useNativeWindowOwner;
            params.window.showOnInitialize = showOnInitialize;

            params.linuxChromeless.resizeBorderThickness = 8;

            params.geometry.left = location.x;
            params.geometry.top = location.y;
            params.geometry.width = size.width;
            params.geometry.height = size.height;
            params.geometry.minWidth = minSize.width;
            params.geometry.minHeight = minSize.height;
            params.geometry.maxWidth = maxSize.width;
            params.geometry.maxHeight = maxSize.height;
            params.geometry.windowState = windowState;
            params.geometry.centerOnInitialize = centerOnInitialize;
            params.geometry.resizable = resizable;
            params.geometry.topmost = topmost;
            params.geometry.useOsDefaultLocation = useOsDefaultLocation;
            params.geometry.useOsDefaultSize = useOsDefaultSize;

            params.browser.startString = startString.empty() ? nullptr : startString.c_str();
            params.browser.startUrl = startUrl.empty() ? nullptr : startUrl.c_str();

            params.browser.zoom = zoom;
            params.browser.zoomEnabled = zoomEnabled;
            params.browser.contextMenuEnabled = contextMenuEnabled;
            params.browser.statusBarEnabled = statusBarEnabled;
            params.browser.devToolsEnabled = devToolsEnabled;

            params.browser.grantBrowserPermissions = grantBrowserPermissions;
            params.browser.mediaAutoplayEnabled = mediaAutoplayEnabled;
            params.browser.fileSystemAccessEnabled = fileSystemAccessEnabled;
            params.browser.webSecurityEnabled = webSecurityEnabled;
            params.browser.javascriptClipboardAccessEnabled = javascriptClipboardAccessEnabled;
            params.browser.mediaStreamEnabled = mediaStreamEnabled;
            params.browser.smoothScrollingEnabled = smoothScrollingEnabled;
            params.browser.ignoreCertificateErrorsEnabled = ignoreCertificateErrorsEnabled;

            return params;
        }

        void ValidateStartupParameters() const
        {
            if (startString.empty() && startUrl.empty())
            {
                throw std::invalid_argument("An initial URL or HTML string must be supplied with Load or LoadString.");
            }

            if (!startString.empty() && !startUrl.empty())
                throw std::invalid_argument("StartString and StartUrl cannot be specified at the same time.");

            if (!IsValidWindowState(windowState))
                throw std::invalid_argument("windowState");

            if (centerOnInitialize && useOsDefaultLocation)
                throw std::invalid_argument("CenterOnInitialize cannot be used with UseOsDefaultLocation.");

            if (size.width < 0)
                throw std::invalid_argument("width");

            if (size.height < 0)
                throw std::invalid_argument("height");

            if (minSize.width < 0)
                throw std::invalid_argument("minWidth");

            if (minSize.height < 0)
                throw std::invalid_argument("minHeight");

            if (maxSize.width < 0)
                throw std::invalid_argument("maxWidth");

            if (maxSize.height < 0)
                throw std::invalid_argument("maxHeight");

            if (minSize.width > maxSize.width)
                throw std::invalid_argument("minWidth");

            if (minSize.height > maxSize.height)
                throw std::invalid_argument("minHeight");
#ifdef _WIN32
            if (chromeless && (useOsDefaultLocation || useOsDefaultSize))
            {
                throw std::invalid_argument(
                    "Chromeless cannot be used with UseOsDefaultLocation or "
                    "UseOsDefaultSize on Windows. Size and location must be specified.");
            }
#endif
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

        void ThrowIfClosedOrInitialized(std::string_view memberName) const
        {
            ThrowIfClosed(memberName);
            ThrowIfInitialized(memberName);
        }

        void ThrowIfClosedOrNotInitialized(std::string_view memberName) const
        {
            ThrowIfClosed(memberName);
            ThrowIfNotInitialized(memberName);
        }

        [[nodiscard]] native::Library& NativeLibrary() const noexcept
        {
            return application.NativeLibrary();
        }

    private:

        template<typename TCallback>
        static void InvokeEvent(Application& application, TCallback&& callback) noexcept
        {
            try
            {
                std::forward<TCallback>(callback)();
            }
            catch (...)
            {
                application.OnUnhandledException(std::current_exception());
            }
        }

        template<typename TCallback>
        static bool InvokeCancelableEvent(Application& application, TCallback&& callback) noexcept
        {
            try
            {
                return std::forward<TCallback>(callback)();
            }
            catch (...)
            {
                application.OnUnhandledException(std::current_exception());
                return false;
            }
        }

        static void CreatedCallback(void* instance, bool registered, void* state) noexcept
        {
            assert(instance);

            auto& window = *static_cast<Window*>(state);
            auto& impl = *window.impl_;

            assert(!impl.nativeInstance);

            impl.nativeInstance = instance;
            impl.application.OnWindowCreated(window, registered);

            InvokeEvent(impl.application, [&impl]
            {
                impl.createdHandlers();
            });
        }

        static bool ClosingCallback(void* state) noexcept
        {
            auto& window = *static_cast<Window*>(state);
            auto& impl = *window.impl_;

            if (impl.forceClose)
                return false;

            return InvokeCancelableEvent(impl.application, [&impl]
            {
                ClosingEventArgs args;
                impl.closingHandlers(args);
                return args.cancel;
            });
        }

        static void ClosedCallback(void* state) noexcept
        {
            auto& window = *static_cast<Window*>(state);
            auto& impl = *window.impl_;

            assert(impl.nativeInstance);

            impl.nativeInstance = nullptr;
            impl.isClosed = true;
            impl.forceClose = false;

            InvokeEvent(impl.application, [&impl]
            {
                impl.closedHandlers();
            });

            impl.application.OnWindowClosed(window);
        }

        static void FocusInCallback(void* state) noexcept
        {
            auto& window = *static_cast<Window*>(state);
            auto& impl = *window.impl_;

            InvokeEvent(impl.application, [&impl]
            {
                impl.activatedHandlers();
            });
        }

        static void FocusOutCallback(void* state) noexcept
        {
            auto& window = *static_cast<Window*>(state);
            auto& impl = *window.impl_;

            InvokeEvent(impl.application, [&impl]
            {
                impl.deactivatedHandlers();
            });
        }

        static void ResizedCallback(int width, int height, void* state) noexcept
        {
            auto& window = *static_cast<Window*>(state);
            auto& impl = *window.impl_;

            const Size size
            {
                .width = width,
                .height = height
            };

            impl.size = size;

            InvokeEvent(impl.application, [&impl, size]
            {
                const SizeChangedEventArgs args
                {
                    .size = size
                };

                impl.sizeChangedHandlers(args);
            });
        }

        static void MovedCallback(int x, int y, void* state) noexcept
        {
            auto& window = *static_cast<Window*>(state);
            auto& impl = *window.impl_;

            const Point location
            {
                .x = x,
                .y = y
            };

            impl.location = location;

            InvokeEvent(impl.application, [&impl, location]
            {
                const LocationChangedEventArgs args
                {
                    .location = location
                };

                impl.locationChangedHandlers(args);
            });
        }

        static void MaximizedCallback(void* state) noexcept
        {
            auto& window = *static_cast<Window*>(state);
            auto& impl = *window.impl_;

            InvokeEvent(impl.application, [&impl]
            {
                impl.maximizedHandlers();
            });
        }

        static void RestoredCallback(void* state) noexcept
        {
            auto& window = *static_cast<Window*>(state);
            auto& impl = *window.impl_;

            InvokeEvent(impl.application, [&impl]
            {
                impl.restoredHandlers();
            });
        }

        static void MinimizedCallback(void* state) noexcept
        {
            auto& window = *static_cast<Window*>(state);
            auto& impl = *window.impl_;

            InvokeEvent(impl.application, [&impl]
            {
                impl.minimizedHandlers();
            });
        }

        static void FullScreenChangedCallback(bool fullScreen, void* state) noexcept
        {
            auto& window = *static_cast<Window*>(state);
            auto& impl = *window.impl_;

            InvokeEvent(impl.application, [&impl, fullScreen]
            {
                if (fullScreen)
                    impl.fullScreenEnteredHandlers();
                else
                    impl.fullScreenExitedHandlers();
            });
        }

        static void StateChangedCallback(WindowState oldState, WindowState newState, void* state) noexcept
        {
            auto& window = *static_cast<Window*>(state);
            auto& impl = *window.impl_;

            impl.windowState = newState;

            InvokeEvent(impl.application, [&impl, oldState, newState]
            {
                const StateChangedEventArgs args
                {
                    .oldState = oldState,
                    .newState = newState
                };

                impl.stateChangedHandlers(args);
            });
        }

        static void WebMessageReceivedCallback(native::Utf8String message, native::Utf8String uri, void* state) noexcept
        {
            auto& window = *static_cast<Window*>(state);
            auto& impl = *window.impl_;

            if (!message || !*message)
            {
                assert(false);
                return;
            }

            InvokeEvent(impl.application, [&impl, message, uri]
            {
                const WebMessageReceivedEventArgs args
                {
                    .message = message,
                    .uri = uri ? uri : ""
                };

                impl.webMessageReceivedHandlers(args);
            });
        }

        static bool NavigationStartingCallback(native::Utf8String uri, void* state) noexcept
        {
            auto& window = *static_cast<Window*>(state);
            auto& impl = *window.impl_;

            return InvokeCancelableEvent(impl.application, [&impl, uri]
            {
                NavigationStartingEventArgs args
                {
                    .uri = uri ? uri : ""
                };

                impl.navigationStartingHandlers(args);
                return args.cancel;
            });
        }

        static bool NewWindowRequestedCallback(native::Utf8String uri, void* state) noexcept
        {
            auto& window = *static_cast<Window*>(state);
            auto& impl = *window.impl_;

            return InvokeCancelableEvent(impl.application, [&impl, uri]
            {
                NewWindowRequestedEventArgs args
                {
                    .uri = uri ? uri : ""
                };

                impl.newWindowRequestedHandlers(args);
                return true; // PhotinoX suppresses browser-controlled popup windows by default.
            });
        }

        static void ContentLoadingCallback(native::Utf8String uri, void* state) noexcept
        {
            auto& window = *static_cast<Window*>(state);
            auto& impl = *window.impl_;

            InvokeEvent(impl.application, [&impl, uri]
            {
                const ContentLoadingEventArgs args
                {
                    .uri = uri ? uri : ""
                };

                impl.contentLoadingHandlers(args);
            });
        }

        static void ContentLoadedCallback(native::Utf8String uri, void* state) noexcept
        {
            auto& window = *static_cast<Window*>(state);
            auto& impl = *window.impl_;

            InvokeEvent(impl.application, [&impl, uri]
            {
                const ContentLoadedEventArgs args
                {
                    .uri = uri ? uri : ""
                };

                if (!impl.initialContentLoadedRaised)
                {
                    impl.initialContentLoadedRaised = true;

                    try
                    {
                        impl.initialContentLoadedHandlers(args);
                    }
                    catch (...)
                    {
                        impl.application.OnUnhandledException(std::current_exception());
                    }
                }

                impl.contentLoadedHandlers(args);
            });
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

    std::string Window::Title() const
    {
        if (!impl_->nativeInstance)
            return impl_->title;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetTitle(impl_->nativeInstance);
        });
    }

    Window& Window::SetTitle(std::string_view title)
    {
        impl_->ThrowIfClosed("SetTitle");

        if (!impl_->nativeInstance)
        {
            impl_->title = title;
            return *this;
        }

        impl_->title = GetDispatcher().Invoke([this, title = std::string(title)]
        {
            auto& library = impl_->NativeLibrary();

            library.WindowSetTitle(impl_->nativeInstance, title.c_str());
            return library.WindowGetTitle(impl_->nativeInstance);
        });

        return *this;
    }

    // IconFile

    std::string Window::IconFile() const
    {
        if (!impl_->nativeInstance)
            return impl_->iconFile;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetIconFile(impl_->nativeInstance);
        });
    }

    Window& Window::SetIconFile(std::string_view iconFile)
    {
        impl_->ThrowIfClosed("SetIconFile");

        if (iconFile.empty())
        {
            if (impl_->nativeInstance)
                throw std::logic_error("SetIconFile cannot clear the icon after the window has been initialized.");

            impl_->iconFile.clear();
            return *this;
        }

        if (!impl_->nativeInstance)
        {
            impl_->iconFile = iconFile;
            return *this;
        }

        impl_->iconFile = GetDispatcher().Invoke([this, iconFile = std::string(iconFile)]
        {
            auto& library = impl_->NativeLibrary();

            library.WindowSetIconFile(impl_->nativeInstance, iconFile.c_str());
            return library.WindowGetIconFile(impl_->nativeInstance);
        });

        return *this;
    }

    // UseOsDefaultSize

    bool Window::UseOsDefaultSize() const noexcept
    {
        return impl_->useOsDefaultSize;
    }

    Window& Window::SetUseOsDefaultSize(bool useDefault)
    {
        impl_->ThrowIfClosedOrInitialized("SetUseOsDefaultSize");
        impl_->useOsDefaultSize = useDefault;
        return *this;
    }

    // UseOsDefaultLocation

    bool Window::UseOsDefaultLocation() const noexcept
    {
        return impl_->useOsDefaultLocation;
    }

    Window& Window::SetUseOsDefaultLocation(bool useDefault)
    {
        impl_->ThrowIfClosedOrInitialized("SetUseOsDefaultLocation");

        impl_->useOsDefaultLocation = useDefault;

        if (useDefault)
            impl_->centerOnInitialize = false;

        return *this;
    }

    // CenterOnInitialize

    bool Window::CenterOnInitialize() const noexcept
    {
        return impl_->centerOnInitialize;
    }

    Window& Window::SetCenterOnInitialize(bool center)
    {
        impl_->ThrowIfClosedOrInitialized("SetCenterOnInitialize");

        impl_->centerOnInitialize = center;

        if (center)
            impl_->useOsDefaultLocation = false;

        return *this;
    }

    // UseNativeWindowOwner

    bool Window::UseNativeWindowOwner() const noexcept
    {
        return impl_->useNativeWindowOwner;
    }

    Window& Window::SetUseNativeWindowOwner(bool useNativeWindowOwner)
    {
        impl_->ThrowIfClosedOrInitialized("SetUseNativeWindowOwner");
        impl_->useNativeWindowOwner = useNativeWindowOwner;
        return *this;
    }

    // Geometry

    // Size

    Size Window::GetSize() const
    {
        if (!impl_->nativeInstance)
            return impl_->size;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetSize(impl_->nativeInstance);
        });
    }

    Window& Window::SetSize(Size size)
    {
        impl_->ThrowIfClosed("SetSize");

        if (!impl_->nativeInstance)
        {
            impl_->size = size;
            impl_->useOsDefaultSize = false;
            return *this;
        }

        GetDispatcher().Invoke([this, size]
        {
            impl_->NativeLibrary().WindowSetSize(impl_->nativeInstance, size);
        });

        return *this;
    }

    Window& Window::SetSize(int width, int height)
    {
        return SetSize(Size{ width, height });
    }

    // Width

    int Window::Width() const
    {
        return GetSize().width;
    }

    Window& Window::SetWidth(int width)
    {
        Size size = GetSize();

        if (size.width != width)
        {
            size.width = width;
            SetSize(size);
        }

        return *this;
    }

    // Height

    int Window::Height() const
    {
        return GetSize().height;
    }

    Window& Window::SetHeight(int height)
    {
        Size size = GetSize();

        if (size.height != height)
        {
            size.height = height;
            SetSize(size);
        }

        return *this;
    }

    // MinSize

    Size Window::MinSize() const noexcept
    {
        return impl_->minSize;
    }

    Window& Window::SetMinSize(Size size)
    {
        impl_->ThrowIfClosed("SetMinSize");

        if (impl_->minSize == size)
            return *this;

        if (impl_->nativeInstance)
        {
            GetDispatcher().Invoke([this, size]
            {
                impl_->NativeLibrary().WindowSetMinSize(impl_->nativeInstance, size);
            });
        }

        impl_->minSize = size;
        return *this;
    }

    Window& Window::SetMinSize(int width, int height)
    {
        return SetMinSize(Size{ width, height });
    }

    // MinWidth

    int Window::MinWidth() const noexcept
    {
        return impl_->minSize.width;
    }

    Window& Window::SetMinWidth(int width)
    {
        Size size = impl_->minSize;
        size.width = width;
        return SetMinSize(size);
    }

    // MinHeight

    int Window::MinHeight() const noexcept
    {
        return impl_->minSize.height;
    }

    Window& Window::SetMinHeight(int height)
    {
        Size size = impl_->minSize;
        size.height = height;
        return SetMinSize(size);
    }

    // MaxSize

    Size Window::MaxSize() const noexcept
    {
        return impl_->maxSize;
    }

    Window& Window::SetMaxSize(Size size)
    {
        impl_->ThrowIfClosed("SetMaxSize");

        if (impl_->maxSize == size)
            return *this;

        if (impl_->nativeInstance)
        {
            GetDispatcher().Invoke([this, size]
            {
                impl_->NativeLibrary().WindowSetMaxSize(impl_->nativeInstance, size);
            });
        }

        impl_->maxSize = size;
        return *this;
    }

    Window& Window::SetMaxSize(int width, int height)
    {
        return SetMaxSize(Size{ width, height });
    }

    // MaxWidth

    int Window::MaxWidth() const noexcept
    {
        return impl_->maxSize.width;
    }

    Window& Window::SetMaxWidth(int width)
    {
        Size size = impl_->maxSize;
        size.width = width;
        return SetMaxSize(size);
    }

    // MaxHeight

    int Window::MaxHeight() const noexcept
    {
        return impl_->maxSize.height;
    }

    Window& Window::SetMaxHeight(int height)
    {
        Size size = impl_->maxSize;
        size.height = height;
        return SetMaxSize(size);
    }

    // Location

    Point Window::Location() const
    {
        if (!impl_->nativeInstance)
            return impl_->location;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetPosition(impl_->nativeInstance);
        });
    }

    Window& Window::SetLocation(Point location)
    {
        impl_->ThrowIfClosed("SetLocation");

        if (!impl_->nativeInstance)
        {
            impl_->location = location;
            impl_->useOsDefaultLocation = false;
            impl_->centerOnInitialize = false;
            return *this;
        }

        GetDispatcher().Invoke([this, location]
        {
            impl_->NativeLibrary().WindowSetPosition(impl_->nativeInstance, location);
        });

        return *this;
    }

    Window& Window::SetLocation(int left, int top)
    {
        return SetLocation(Point{ left, top });
    }

    // Left

    int Window::Left() const
    {
        return Location().x;
    }

    Window& Window::SetLeft(int left)
    {
        Point location = Location();

        if (location.x != left)
        {
            location.x = left;
            SetLocation(location);
        }

        return *this;
    }

    // Top

    int Window::Top() const
    {
        return Location().y;
    }

    Window& Window::SetTop(int top)
    {
        Point location = Location();

        if (location.y != top)
        {
            location.y = top;
            SetLocation(location);
        }

        return *this;
    }

    Window& Window::Center()
    {
        impl_->ThrowIfClosed("Center");

        if (!impl_->nativeInstance)
        {
            impl_->centerOnInitialize = true;
            impl_->useOsDefaultLocation = false;
            return *this;
        }

        const bool centered = GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowCenter(impl_->nativeInstance);
        });

        if (!centered)
            throw std::runtime_error("Failed to center the window.");

        return *this;
    }

    // Window state

    WindowState Window::GetWindowState() const
    {
        if (!impl_->nativeInstance)
            return impl_->windowState;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetState(impl_->nativeInstance);
        });
    }

    Window& Window::SetWindowState(WindowState state)
    {
        impl_->ThrowIfClosed("SetWindowState");

        if (!IsValidWindowState(state))
            throw std::invalid_argument("state");

        if (!impl_->nativeInstance)
        {
            impl_->windowState = state;
            return *this;
        }

        GetDispatcher().Invoke([this, state]
        {
            impl_->NativeLibrary().WindowSetState(impl_->nativeInstance, state);
        });

        return *this;
    }

    // Maximized

    bool Window::Maximized() const
    {
        if (!impl_->nativeInstance)
            return impl_->windowState == WindowState::Maximized;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetMaximized(impl_->nativeInstance);
        });
    }

    Window& Window::SetMaximized(bool maximized)
    {
        impl_->ThrowIfClosed("SetMaximized");

        if (maximized)
            return Maximize();

        if (!impl_->nativeInstance)
        {
            if (impl_->windowState == WindowState::Maximized)
                impl_->windowState = WindowState::Normal;

            return *this;
        }

        GetDispatcher().Invoke([this]
        {
            impl_->NativeLibrary().WindowSetMaximized(impl_->nativeInstance, false);
        });

        return *this;
    }

    // Minimized

    bool Window::Minimized() const
    {
        if (!impl_->nativeInstance)
            return impl_->windowState == WindowState::Minimized;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetMinimized(impl_->nativeInstance);
        });
    }

    Window& Window::SetMinimized(bool minimized)
    {
        impl_->ThrowIfClosed("SetMinimized");

        if (minimized)
            return Minimize();

        if (!impl_->nativeInstance)
        {
            if (impl_->windowState == WindowState::Minimized)
                impl_->windowState = WindowState::Normal;

            return *this;
        }

        GetDispatcher().Invoke([this]
        {
            impl_->NativeLibrary().WindowSetMinimized(impl_->nativeInstance, false);
        });

        return *this;
    }

    // FullScreen

    bool Window::FullScreen() const
    {
        if (!impl_->nativeInstance)
            return impl_->windowState == WindowState::FullScreen;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetFullScreen(impl_->nativeInstance);
        });
    }

    Window& Window::SetFullScreen(bool fullScreen)
    {
        impl_->ThrowIfClosed("SetFullScreen");

        if (!impl_->nativeInstance)
        {
            if (fullScreen)
                impl_->windowState = WindowState::FullScreen;
            else if (impl_->windowState == WindowState::FullScreen)
                impl_->windowState = WindowState::Normal;

            return *this;
        }

        GetDispatcher().Invoke([this, fullScreen]
        {
            impl_->NativeLibrary().WindowSetFullScreen(impl_->nativeInstance, fullScreen);
        });

        return *this;
    }

    // Resizable

    bool Window::Resizable() const
    {
        if (!impl_->nativeInstance)
            return impl_->resizable;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetResizable(impl_->nativeInstance);
        });
    }

    Window& Window::SetResizable(bool resizable)
    {
        impl_->ThrowIfClosed("SetResizable");

        if (!impl_->nativeInstance)
        {
            impl_->resizable = resizable;
            return *this;
        }

        GetDispatcher().Invoke([this, resizable]
        {
            impl_->NativeLibrary().WindowSetResizable(impl_->nativeInstance, resizable);
        });

        return *this;
    }

    // Topmost

    bool Window::Topmost() const
    {
        if (!impl_->nativeInstance)
            return impl_->topmost;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetTopmost(impl_->nativeInstance);
        });
    }

    Window& Window::SetTopmost(bool topmost)
    {
        impl_->ThrowIfClosed("SetTopmost");

        if (!impl_->nativeInstance)
        {
            impl_->topmost = topmost;
            return *this;
        }

        GetDispatcher().Invoke([this, topmost]
        {
            impl_->NativeLibrary().WindowSetTopmost(impl_->nativeInstance, topmost);
        });

        return *this;
    }

    bool Window::Activate()
    {
        impl_->ThrowIfClosedOrNotInitialized("Activate");

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowActivate(impl_->nativeInstance);
        });
    }

    Window& Window::BringToFront()
    {
        impl_->ThrowIfClosed("BringToFront");

        Show();

        if (GetWindowState() == WindowState::Minimized)
            Restore();

        if (!Activate())
            throw std::runtime_error("Failed to activate the window.");

        return *this;
    }

    Window& Window::Maximize()
    {
        impl_->ThrowIfClosed("Maximize");

        if (!impl_->nativeInstance)
        {
            impl_->windowState = WindowState::Maximized;
            return *this;
        }

        const bool maximized = GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowMaximize(impl_->nativeInstance);
        });

        if (!maximized)
            throw std::runtime_error("Failed to maximize the window.");

        return *this;
    }

    Window& Window::Minimize()
    {
        impl_->ThrowIfClosed("Minimize");

        if (!impl_->nativeInstance)
        {
            impl_->windowState = WindowState::Minimized;
            return *this;
        }

        const bool minimized = GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowMinimize(impl_->nativeInstance);
        });

        if (!minimized)
            throw std::runtime_error("Failed to minimize the window.");

        return *this;
    }

    Window& Window::Restore()
    {
        impl_->ThrowIfClosed("Restore");

        if (!impl_->nativeInstance)
        {
            impl_->windowState = WindowState::Normal;
            return *this;
        }

        const bool restored = GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowRestore(impl_->nativeInstance);
        });

        if (!restored)
            throw std::runtime_error("Failed to restore the window.");

        return *this;
    }

    // Appearance

    // Chromeless

    bool Window::Chromeless() const noexcept
    {
        return impl_->chromeless;
    }

    Window& Window::SetChromeless(bool chromeless)
    {
        impl_->ThrowIfClosedOrInitialized("SetChromeless");
        impl_->chromeless = chromeless;
        return *this;
    }

    // Transparent

    bool Window::Transparent() const
    {
        if (!impl_->nativeInstance)
            return impl_->transparent;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetTransparentEnabled(impl_->nativeInstance);
        });
    }

    Window& Window::SetTransparent(bool transparent)
    {
        impl_->ThrowIfClosed("SetTransparent");

        if (!impl_->nativeInstance)
        {
            impl_->transparent = transparent;
            return *this;
        }

#if defined(_WIN32) || defined(__APPLE__)
        throw std::runtime_error("SetTransparent cannot be called on Windows or macOS after the window has been initialized.");
#endif

        GetDispatcher().Invoke([this, transparent]
        {
            impl_->NativeLibrary().WindowSetTransparentEnabled(impl_->nativeInstance, transparent);
        });

        return *this;
    }

    // Browser

    // StartString

    std::string_view Window::StartString() const noexcept
    {
        return impl_->startString;
    }

    Window& Window::SetStartString(std::string_view content)
    {
        impl_->ThrowIfClosedOrInitialized("SetStartString");
        impl_->startString = content;
        return *this;
    }

    Window& Window::LoadString(std::string_view content)
    {
        impl_->ThrowIfClosed("LoadString");

        if (!impl_->nativeInstance)
        {
            impl_->startString = content;
            impl_->startUrl.clear();
            return *this;
        }

        GetDispatcher().Invoke([this, content = std::string(content)]
        {
            impl_->NativeLibrary().WindowNavigateToString(impl_->nativeInstance, content.c_str());
        });

        return *this;
    }

    // StartUrl

    std::string_view Window::StartUrl() const noexcept
    {
        return impl_->startUrl;
    }

    Window& Window::SetStartUrl(std::string_view url)
    {
        impl_->ThrowIfClosedOrInitialized("SetStartUrl");
        impl_->startUrl = url;
        return *this;
    }

    Window& Window::Load(std::string_view url)
    {
        impl_->ThrowIfClosed("Load");

        if (url.empty())
            throw std::invalid_argument("url");

        if (!impl_->nativeInstance)
        {
            impl_->startUrl = url;
            impl_->startString.clear();
            return *this;
        }

        GetDispatcher().Invoke([this, url = std::string(url)]
        {
            impl_->NativeLibrary().WindowNavigateToUrl(impl_->nativeInstance, url.c_str());
        });

        return *this;
    }

    // ContextMenuEnabled

    bool Window::ContextMenuEnabled() const
    {
        if (!impl_->nativeInstance)
            return impl_->contextMenuEnabled;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetContextMenuEnabled(impl_->nativeInstance);
        });
    }

    Window& Window::SetContextMenuEnabled(bool enabled)
    {
        impl_->ThrowIfClosed("SetContextMenuEnabled");

        if (!impl_->nativeInstance)
        {
            impl_->contextMenuEnabled = enabled;
            return *this;
        }

        GetDispatcher().Invoke([this, enabled]
        {
            impl_->NativeLibrary().WindowSetContextMenuEnabled(impl_->nativeInstance, enabled);
        });

        return *this;
    }

    // ZoomEnabled

    bool Window::ZoomEnabled() const
    {
        if (!impl_->nativeInstance)
            return impl_->zoomEnabled;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetZoomEnabled(impl_->nativeInstance);
        });
    }

    Window& Window::SetZoomEnabled(bool enabled)
    {
        impl_->ThrowIfClosed("SetZoomEnabled");

        if (!impl_->nativeInstance)
        {
            impl_->zoomEnabled = enabled;
            return *this;
        }

        GetDispatcher().Invoke([this, enabled]
        {
            impl_->NativeLibrary().WindowSetZoomEnabled(impl_->nativeInstance, enabled);
        });

        return *this;
    }

    // StatusBarEnabled

    bool Window::StatusBarEnabled() const
    {
        if (!impl_->nativeInstance)
            return impl_->statusBarEnabled;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetStatusBarEnabled(impl_->nativeInstance);
        });
    }

    Window& Window::SetStatusBarEnabled(bool enabled)
    {
        impl_->ThrowIfClosed("SetStatusBarEnabled");

        if (!impl_->nativeInstance)
        {
            impl_->statusBarEnabled = enabled;
            return *this;
        }

        GetDispatcher().Invoke([this, enabled]
        {
            impl_->NativeLibrary().WindowSetStatusBarEnabled(impl_->nativeInstance, enabled);
        });

        return *this;
    }

    // DevToolsEnabled

    bool Window::DevToolsEnabled() const
    {
        if (!impl_->nativeInstance)
            return impl_->devToolsEnabled;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetDevToolsEnabled(impl_->nativeInstance);
        });
    }

    Window& Window::SetDevToolsEnabled(bool enabled)
    {
        impl_->ThrowIfClosed("SetDevToolsEnabled");

        if (!impl_->nativeInstance)
        {
            impl_->devToolsEnabled = enabled;
            return *this;
        }

        GetDispatcher().Invoke([this, enabled]
        {
            impl_->NativeLibrary().WindowSetDevToolsEnabled(impl_->nativeInstance, enabled);
        });

        return *this;
    }

    int Window::Zoom() const
    {
        if (!impl_->nativeInstance)
            return impl_->zoom;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetZoom(impl_->nativeInstance);
        });
    }

    Window& Window::SetZoom(int zoom)
    {
        impl_->ThrowIfClosed("SetZoom");

        if (!impl_->nativeInstance)
        {
            impl_->zoom = zoom;
            return *this;
        }

        GetDispatcher().Invoke([this, zoom]
        {
            impl_->NativeLibrary().WindowSetZoom(impl_->nativeInstance, zoom);
        });

        return *this;
    }

    // GrantBrowserPermissions

    bool Window::GrantBrowserPermissions() const
    {
        if (!impl_->nativeInstance)
            return impl_->grantBrowserPermissions;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetGrantBrowserPermissions(impl_->nativeInstance);
        });
    }

    Window& Window::SetGrantBrowserPermissions(bool grant)
    {
        impl_->ThrowIfClosedOrInitialized("SetGrantBrowserPermissions");
        impl_->grantBrowserPermissions = grant;
        return *this;
    }

    // MediaAutoplayEnabled

    bool Window::MediaAutoplayEnabled() const
    {
        if (!impl_->nativeInstance)
            return impl_->mediaAutoplayEnabled;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetMediaAutoplayEnabled(impl_->nativeInstance);
        });
    }

    Window& Window::SetMediaAutoplayEnabled(bool enabled)
    {
        impl_->ThrowIfClosedOrInitialized("SetMediaAutoplayEnabled");
        impl_->mediaAutoplayEnabled = enabled;
        return *this;
    }

    // FileSystemAccessEnabled

    bool Window::FileSystemAccessEnabled() const
    {
        if (!impl_->nativeInstance)
            return impl_->fileSystemAccessEnabled;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetFileSystemAccessEnabled(impl_->nativeInstance);
        });
    }

    Window& Window::SetFileSystemAccessEnabled(bool enabled)
    {
        impl_->ThrowIfClosedOrInitialized("SetFileSystemAccessEnabled");
        impl_->fileSystemAccessEnabled = enabled;
        return *this;
    }

    // WebSecurityEnabled

    bool Window::WebSecurityEnabled() const
    {
        if (!impl_->nativeInstance)
            return impl_->webSecurityEnabled;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetWebSecurityEnabled(impl_->nativeInstance);
        });
    }

    Window& Window::SetWebSecurityEnabled(bool enabled)
    {
        impl_->ThrowIfClosedOrInitialized("SetWebSecurityEnabled");
        impl_->webSecurityEnabled = enabled;
        return *this;
    }

    bool Window::JavascriptClipboardAccessEnabled() const
    {
        if (!impl_->nativeInstance)
            return impl_->javascriptClipboardAccessEnabled;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetJavascriptClipboardAccessEnabled(impl_->nativeInstance);
        });
    }

    Window& Window::SetJavascriptClipboardAccessEnabled(bool enabled)
    {
        impl_->ThrowIfClosedOrInitialized("SetJavascriptClipboardAccessEnabled");
        impl_->javascriptClipboardAccessEnabled = enabled;
        return *this;
    }

    // MediaStreamEnabled

    bool Window::MediaStreamEnabled() const
    {
        if (!impl_->nativeInstance)
            return impl_->mediaStreamEnabled;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetMediaStreamEnabled(impl_->nativeInstance);
        });
    }

    Window& Window::SetMediaStreamEnabled(bool enabled)
    {
        impl_->ThrowIfClosedOrInitialized("SetMediaStreamEnabled");
        impl_->mediaStreamEnabled = enabled;
        return *this;
    }

    // SmoothScrollingEnabled

    bool Window::SmoothScrollingEnabled() const
    {
        if (!impl_->nativeInstance)
            return impl_->smoothScrollingEnabled;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetSmoothScrollingEnabled(impl_->nativeInstance);
        });
    }

    Window& Window::SetSmoothScrollingEnabled(bool enabled)
    {
        impl_->ThrowIfClosedOrInitialized("SetSmoothScrollingEnabled");
        impl_->smoothScrollingEnabled = enabled;
        return *this;
    }

    // IgnoreCertificateErrorsEnabled

    bool Window::IgnoreCertificateErrorsEnabled() const
    {
        if (!impl_->nativeInstance)
            return impl_->ignoreCertificateErrorsEnabled;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetIgnoreCertificateErrorsEnabled(impl_->nativeInstance);
        });
    }

    Window& Window::SetIgnoreCertificateErrorsEnabled(bool enabled)
    {
        impl_->ThrowIfClosedOrInitialized("SetIgnoreCertificateErrorsEnabled");
        impl_->ignoreCertificateErrorsEnabled = enabled;
        return *this;
    }

    // Features

    Window& Window::ClearBrowserAutoFill()
    {
        impl_->ThrowIfClosedOrNotInitialized("ClearBrowserAutoFill");

#ifndef _WIN32
        throw std::runtime_error("ClearBrowserAutoFill is only supported on Windows.");
#endif

        GetDispatcher().Invoke([this]
        {
            impl_->NativeLibrary().WindowClearBrowserAutoFill(impl_->nativeInstance);
        });

        return *this;
    }

    // Communication

    Window& Window::SendWebMessage(std::string_view message)
    {
        impl_->ThrowIfClosedOrNotInitialized("SendWebMessage");

        GetDispatcher().Invoke([this, message = std::string(message)]
        {
            impl_->NativeLibrary().WindowSendWebMessage(impl_->nativeInstance, message.c_str());
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

    bool Window::IsVisible() const
    {
        if (!impl_->nativeInstance)
            return false;

        return GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowGetVisible(impl_->nativeInstance);
        });
    }

    Application& Window::GetApplication() const noexcept
    {
        return impl_->application;
    }

    Dispatcher& Window::GetDispatcher() noexcept
    {
        return impl_->application.GetDispatcher();
    }

    const Dispatcher& Window::GetDispatcher() const noexcept
    {
        return impl_->application.GetDispatcher();
    }

    Window* Window::Parent() const noexcept
    {
        return impl_->parent;
    }

    // Lifecycle methods

    void Window::InternalClose()
    {
        if (!impl_->nativeInstance || impl_->isClosed)
            return;

        impl_->forceClose = true;
        Close();
    }

    void Window::Initialize()
    {
        InitializeCore(false);
    }

    void Window::InitializeCore(bool showOnInitialize)
    {
        impl_->ThrowIfClosed("Initialize");

        if (impl_->nativeInstance)
            return;

        if (impl_->isCreating)
            throw std::logic_error("The window is already being created.");

        GetDispatcher().VerifyAccessToCreateWindow();

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

        impl_->ValidateStartupParameters();

        auto params = impl_->CreateInitParams(this, showOnInitialize);
        void* nativeInstance = impl_->NativeLibrary().WindowCreate(&params);

        assert(impl_->nativeInstance == nativeInstance);

        if (!nativeInstance)
            throw std::runtime_error("Native window creation failed.");

        impl_->nativeInstance = nativeInstance;
    }

    void Window::Show()
    {
        impl_->ThrowIfClosed("Show");

        if (!impl_->nativeInstance)
        {
            InitializeCore(true);
            return;
        }

        const bool shown = GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowShow(impl_->nativeInstance);
        });

        assert(shown);

        if (!shown)
            throw std::runtime_error("Failed to show the window.");
    }

    void Window::Hide()
    {
        impl_->ThrowIfClosed("Hide");

        if (!impl_->nativeInstance)
            return;

        const bool hidden = GetDispatcher().Invoke([this]
        {
            return impl_->NativeLibrary().WindowHide(impl_->nativeInstance);
        });

        assert(hidden);

        if (!hidden)
            throw std::runtime_error("Failed to hide the window.");
    }

    void Window::Close()
    {
        impl_->ThrowIfClosedOrNotInitialized("Close");

        GetDispatcher().Invoke([this]
        {
            impl_->NativeLibrary().WindowClose(impl_->nativeInstance);
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

    // Activated Handlers

    Window& Window::RegisterActivatedHandler(WindowHandler handler)
    {
        impl_->ThrowIfClosed("RegisterActivatedHandler");
        RegisterEventHandler(impl_->activatedHandlers, std::move(handler));
        return *this;
    }

    EventToken Window::SubscribeActivatedHandler(WindowHandler handler)
    {
        impl_->ThrowIfClosed("SubscribeActivatedHandler");
        return impl_->eventSubscriptions.Subscribe(impl_->activatedHandlers, impl_->activatedHandlerSubscriptions, std::move(handler));
    }

    bool Window::UnsubscribeActivatedHandler(EventToken token)
    {
        return impl_->eventSubscriptions.Unsubscribe(impl_->activatedHandlers, impl_->activatedHandlerSubscriptions, token);
    }

    // Deactivated Handlers

    Window& Window::RegisterDeactivatedHandler(WindowHandler handler)
    {
        impl_->ThrowIfClosed("RegisterDeactivatedHandler");
        RegisterEventHandler(impl_->deactivatedHandlers, std::move(handler));
        return *this;
    }

    EventToken Window::SubscribeDeactivatedHandler(WindowHandler handler)
    {
        impl_->ThrowIfClosed("SubscribeDeactivatedHandler");
        return impl_->eventSubscriptions.Subscribe(impl_->deactivatedHandlers, impl_->deactivatedHandlerSubscriptions, std::move(handler));
    }

    bool Window::UnsubscribeDeactivatedHandler(EventToken token)
    {
        return impl_->eventSubscriptions.Unsubscribe(impl_->deactivatedHandlers, impl_->deactivatedHandlerSubscriptions, token);
    }

    // SizeChanged Handlers

    Window& Window::RegisterSizeChangedHandler(SizeChangedHandler handler)
    {
        impl_->ThrowIfClosed("RegisterSizeChangedHandler");
        RegisterEventHandler(impl_->sizeChangedHandlers, std::move(handler));
        return *this;
    }

    EventToken Window::SubscribeSizeChangedHandler(SizeChangedHandler handler)
    {
        impl_->ThrowIfClosed("SubscribeSizeChangedHandler");
        return impl_->eventSubscriptions.Subscribe(impl_->sizeChangedHandlers, impl_->sizeChangedHandlerSubscriptions, std::move(handler));
    }

    bool Window::UnsubscribeSizeChangedHandler(EventToken token)
    {
        return impl_->eventSubscriptions.Unsubscribe(impl_->sizeChangedHandlers, impl_->sizeChangedHandlerSubscriptions, token);
    }

    // LocationChanged Handlers

    Window& Window::RegisterLocationChangedHandler(LocationChangedHandler handler)
    {
        impl_->ThrowIfClosed("RegisterLocationChangedHandler");
        RegisterEventHandler(impl_->locationChangedHandlers, std::move(handler));
        return *this;
    }

    EventToken Window::SubscribeLocationChangedHandler(LocationChangedHandler handler)
    {
        impl_->ThrowIfClosed("SubscribeLocationChangedHandler");
        return impl_->eventSubscriptions.Subscribe(impl_->locationChangedHandlers, impl_->locationChangedHandlerSubscriptions, std::move(handler));
    }

    bool Window::UnsubscribeLocationChangedHandler(EventToken token)
    {
        return impl_->eventSubscriptions.Unsubscribe(impl_->locationChangedHandlers, impl_->locationChangedHandlerSubscriptions, token);
    }

    // Maximized Handlers

    Window& Window::RegisterMaximizedHandler(WindowHandler handler)
    {
        impl_->ThrowIfClosed("RegisterMaximizedHandler");
        RegisterEventHandler(impl_->maximizedHandlers, std::move(handler));
        return *this;
    }

    EventToken Window::SubscribeMaximizedHandler(WindowHandler handler)
    {
        impl_->ThrowIfClosed("SubscribeMaximizedHandler");
        return impl_->eventSubscriptions.Subscribe(impl_->maximizedHandlers, impl_->maximizedHandlerSubscriptions, std::move(handler));
    }

    bool Window::UnsubscribeMaximizedHandler(EventToken token)
    {
        return impl_->eventSubscriptions.Unsubscribe(impl_->maximizedHandlers, impl_->maximizedHandlerSubscriptions, token);
    }

    // Restored Handlers

    Window& Window::RegisterRestoredHandler(WindowHandler handler)
    {
        impl_->ThrowIfClosed("RegisterRestoredHandler");
        RegisterEventHandler(impl_->restoredHandlers, std::move(handler));
        return *this;
    }

    EventToken Window::SubscribeRestoredHandler(WindowHandler handler)
    {
        impl_->ThrowIfClosed("SubscribeRestoredHandler");
        return impl_->eventSubscriptions.Subscribe(impl_->restoredHandlers, impl_->restoredHandlerSubscriptions, std::move(handler));
    }

    bool Window::UnsubscribeRestoredHandler(EventToken token)
    {
        return impl_->eventSubscriptions.Unsubscribe(impl_->restoredHandlers, impl_->restoredHandlerSubscriptions, token);
    }

    // Minimized Handlers

    Window& Window::RegisterMinimizedHandler(WindowHandler handler)
    {
        impl_->ThrowIfClosed("RegisterMinimizedHandler");
        RegisterEventHandler(impl_->minimizedHandlers, std::move(handler));
        return *this;
    }

    EventToken Window::SubscribeMinimizedHandler(WindowHandler handler)
    {
        impl_->ThrowIfClosed("SubscribeMinimizedHandler");
        return impl_->eventSubscriptions.Subscribe(impl_->minimizedHandlers, impl_->minimizedHandlerSubscriptions, std::move(handler));
    }

    bool Window::UnsubscribeMinimizedHandler(EventToken token)
    {
        return impl_->eventSubscriptions.Unsubscribe(impl_->minimizedHandlers, impl_->minimizedHandlerSubscriptions, token);
    }

    // FullScreenEntered Handlers

    Window& Window::RegisterFullScreenEnteredHandler(WindowHandler handler)
    {
        impl_->ThrowIfClosed("RegisterFullScreenEnteredHandler");
        RegisterEventHandler(impl_->fullScreenEnteredHandlers, std::move(handler));
        return *this;
    }

    EventToken Window::SubscribeFullScreenEnteredHandler(WindowHandler handler)
    {
        impl_->ThrowIfClosed("SubscribeFullScreenEnteredHandler");
        return impl_->eventSubscriptions.Subscribe(impl_->fullScreenEnteredHandlers, impl_->fullScreenEnteredHandlerSubscriptions, std::move(handler));
    }

    bool Window::UnsubscribeFullScreenEnteredHandler(EventToken token)
    {
        return impl_->eventSubscriptions.Unsubscribe(impl_->fullScreenEnteredHandlers, impl_->fullScreenEnteredHandlerSubscriptions, token);
    }

    // FullScreenExited Handlers

    Window& Window::RegisterFullScreenExitedHandler(WindowHandler handler)
    {
        impl_->ThrowIfClosed("RegisterFullScreenExitedHandler");
        RegisterEventHandler(impl_->fullScreenExitedHandlers, std::move(handler));
        return *this;
    }

    EventToken Window::SubscribeFullScreenExitedHandler(WindowHandler handler)
    {
        impl_->ThrowIfClosed("SubscribeFullScreenExitedHandler");
        return impl_->eventSubscriptions.Subscribe(impl_->fullScreenExitedHandlers, impl_->fullScreenExitedHandlerSubscriptions, std::move(handler));
    }

    bool Window::UnsubscribeFullScreenExitedHandler(EventToken token)
    {
        return impl_->eventSubscriptions.Unsubscribe(impl_->fullScreenExitedHandlers, impl_->fullScreenExitedHandlerSubscriptions, token);
    }

    // StateChanged Handlers

    Window& Window::RegisterStateChangedHandler(StateChangedHandler handler)
    {
        impl_->ThrowIfClosed("RegisterStateChangedHandler");
        RegisterEventHandler(impl_->stateChangedHandlers, std::move(handler));
        return *this;
    }

    EventToken Window::SubscribeStateChangedHandler(StateChangedHandler handler)
    {
        impl_->ThrowIfClosed("SubscribeStateChangedHandler");
        return impl_->eventSubscriptions.Subscribe(impl_->stateChangedHandlers, impl_->stateChangedHandlerSubscriptions, std::move(handler));
    }

    bool Window::UnsubscribeStateChangedHandler(EventToken token)
    {
        return impl_->eventSubscriptions.Unsubscribe(impl_->stateChangedHandlers, impl_->stateChangedHandlerSubscriptions, token);
    }

    // WebMessageReceived Handlers

    Window& Window::RegisterWebMessageReceivedHandler(WebMessageReceivedHandler handler)
    {
        impl_->ThrowIfClosed("RegisterWebMessageReceivedHandler");
        RegisterEventHandler(impl_->webMessageReceivedHandlers, std::move(handler));
        return *this;
    }

    EventToken Window::SubscribeWebMessageReceivedHandler(WebMessageReceivedHandler handler)
    {
        impl_->ThrowIfClosed("SubscribeWebMessageReceivedHandler");
        return impl_->eventSubscriptions.Subscribe(impl_->webMessageReceivedHandlers, impl_->webMessageReceivedHandlerSubscriptions, std::move(handler));
    }

    bool Window::UnsubscribeWebMessageReceivedHandler(EventToken token)
    {
        return impl_->eventSubscriptions.Unsubscribe(impl_->webMessageReceivedHandlers, impl_->webMessageReceivedHandlerSubscriptions, token);
    }

    // NavigationStarting Handlers

    Window& Window::RegisterNavigationStartingHandler(NavigationStartingHandler handler)
    {
        impl_->ThrowIfClosed("RegisterNavigationStartingHandler");
        RegisterEventHandler(impl_->navigationStartingHandlers, std::move(handler));
        return *this;
    }

    EventToken Window::SubscribeNavigationStartingHandler(NavigationStartingHandler handler)
    {
        impl_->ThrowIfClosed("SubscribeNavigationStartingHandler");
        return impl_->eventSubscriptions.Subscribe(impl_->navigationStartingHandlers, impl_->navigationStartingHandlerSubscriptions, std::move(handler));
    }

    bool Window::UnsubscribeNavigationStartingHandler(EventToken token)
    {
        return impl_->eventSubscriptions.Unsubscribe(impl_->navigationStartingHandlers, impl_->navigationStartingHandlerSubscriptions, token);
    }

    // NewWindowRequested Handlers

    Window& Window::RegisterNewWindowRequestedHandler(NewWindowRequestedHandler handler)
    {
        impl_->ThrowIfClosed("RegisterNewWindowRequestedHandler");
        RegisterEventHandler(impl_->newWindowRequestedHandlers, std::move(handler));
        return *this;
    }

    EventToken Window::SubscribeNewWindowRequestedHandler(NewWindowRequestedHandler handler)
    {
        impl_->ThrowIfClosed("SubscribeNewWindowRequestedHandler");
        return impl_->eventSubscriptions.Subscribe(impl_->newWindowRequestedHandlers,  impl_->newWindowRequestedHandlerSubscriptions,  std::move(handler));
    }

    bool Window::UnsubscribeNewWindowRequestedHandler(EventToken token)
    {
        return impl_->eventSubscriptions.Unsubscribe(impl_->newWindowRequestedHandlers, impl_->newWindowRequestedHandlerSubscriptions, token);
    }

    // ContentLoading Handlers

    Window& Window::RegisterContentLoadingHandler(ContentLoadingHandler handler)
    {
        impl_->ThrowIfClosed("RegisterContentLoadingHandler");
        RegisterEventHandler(impl_->contentLoadingHandlers, std::move(handler));
        return *this;
    }

    EventToken Window::SubscribeContentLoadingHandler(ContentLoadingHandler handler)
    {
        impl_->ThrowIfClosed("SubscribeContentLoadingHandler");
        return impl_->eventSubscriptions.Subscribe(impl_->contentLoadingHandlers, impl_->contentLoadingHandlerSubscriptions, std::move(handler));
    }

    bool Window::UnsubscribeContentLoadingHandler(EventToken token)
    {
        return impl_->eventSubscriptions.Unsubscribe(impl_->contentLoadingHandlers, impl_->contentLoadingHandlerSubscriptions, token);
    }

    // ContentLoaded Handlers

    Window& Window::RegisterContentLoadedHandler(ContentLoadedHandler handler)
    {
        impl_->ThrowIfClosed("RegisterContentLoadedHandler");
        RegisterEventHandler(impl_->contentLoadedHandlers, std::move(handler));
        return *this;
    }

    EventToken Window::SubscribeContentLoadedHandler(ContentLoadedHandler handler)
    {
        impl_->ThrowIfClosed("SubscribeContentLoadedHandler");
        return impl_->eventSubscriptions.Subscribe(impl_->contentLoadedHandlers, impl_->contentLoadedHandlerSubscriptions, std::move(handler));
    }

    bool Window::UnsubscribeContentLoadedHandler(EventToken token)
    {
        return impl_->eventSubscriptions.Unsubscribe(impl_->contentLoadedHandlers, impl_->contentLoadedHandlerSubscriptions, token);
    }

    // InitialContentLoaded Handlers

    Window& Window::RegisterInitialContentLoadedHandler(ContentLoadedHandler handler)
    {
        impl_->ThrowIfClosed("RegisterInitialContentLoadedHandler");
        RegisterEventHandler(impl_->initialContentLoadedHandlers, std::move(handler));
        return *this;
    }

    EventToken Window::SubscribeInitialContentLoadedHandler(ContentLoadedHandler handler)
    {
        impl_->ThrowIfClosed("SubscribeInitialContentLoadedHandler");
        return impl_->eventSubscriptions.Subscribe(impl_->initialContentLoadedHandlers, impl_->initialContentLoadedHandlerSubscriptions, std::move(handler));
    }

    bool Window::UnsubscribeInitialContentLoadedHandler(EventToken token)
    {
        return impl_->eventSubscriptions.Unsubscribe(impl_->initialContentLoadedHandlers, impl_->initialContentLoadedHandlerSubscriptions, token);
    }
}