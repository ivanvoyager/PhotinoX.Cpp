#pragma once

#include <photinox/callbacks.hpp>
#include <photinox/event_token.hpp>
#include <photinox/geometry.hpp>

#include <memory>
#include <string>
#include <string_view>

namespace photinox
{
    class Application;
    class Dispatcher;

    class Window final
    {
    public:
        explicit Window(Application& application, Window* parent = nullptr);
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        Window(Window&&) = delete;
        Window& operator=(Window&&) = delete;

        // Startup / initialization

        [[nodiscard]] std::string Title() const;
        Window& SetTitle(std::string_view title);

        [[nodiscard]] std::string IconFile() const;
        Window& SetIconFile(std::string_view iconFile);

        [[nodiscard]] bool UseOsDefaultSize() const noexcept;
        Window& SetUseOsDefaultSize(bool useDefault);

        [[nodiscard]] bool UseOsDefaultLocation() const noexcept;
        Window& SetUseOsDefaultLocation(bool useDefault);

        [[nodiscard]] bool CenterOnInitialize() const noexcept;
        Window& SetCenterOnInitialize(bool center);

        [[nodiscard]] bool UseNativeWindowOwner() const noexcept;
        Window& SetUseNativeWindowOwner(bool useNativeWindowOwner);

        // Geometry

        [[nodiscard]] Size GetSize() const;
        Window& SetSize(Size size);
        Window& SetSize(int width, int height);

        [[nodiscard]] int Width() const;
        Window& SetWidth(int width);

        [[nodiscard]] int Height() const;
        Window& SetHeight(int height);

        [[nodiscard]] Size MinSize() const noexcept;
        Window& SetMinSize(Size size);
        Window& SetMinSize(int width, int height);

        [[nodiscard]] int MinWidth() const noexcept;
        Window& SetMinWidth(int width);

        [[nodiscard]] int MinHeight() const noexcept;
        Window& SetMinHeight(int height);

        [[nodiscard]] Size MaxSize() const noexcept;
        Window& SetMaxSize(Size size);
        Window& SetMaxSize(int width, int height);

        [[nodiscard]] int MaxWidth() const noexcept;
        Window& SetMaxWidth(int width);

        [[nodiscard]] int MaxHeight() const noexcept;
        Window& SetMaxHeight(int height);

        [[nodiscard]] Point Location() const;
        Window& SetLocation(Point location);
        Window& SetLocation(int left, int top);

        [[nodiscard]] int Left() const;
        Window& SetLeft(int left);

        [[nodiscard]] int Top() const;
        Window& SetTop(int top);

        Window& Center();

        // Window state

        [[nodiscard]] WindowState GetWindowState() const;
        Window& SetWindowState(WindowState state);

        [[nodiscard]] bool Maximized() const;
        Window& SetMaximized(bool maximized);

        [[nodiscard]] bool Minimized() const;
        Window& SetMinimized(bool minimized);

        [[nodiscard]] bool FullScreen() const;
        Window& SetFullScreen(bool fullScreen);

        [[nodiscard]] bool Resizable() const;
        Window& SetResizable(bool resizable);

        [[nodiscard]] bool Topmost() const;
        Window& SetTopmost(bool topmost);

        [[nodiscard]] bool Activate();
        Window& BringToFront();
        Window& Maximize();
        Window& Minimize();
        Window& Restore();

        // Browser

        [[nodiscard]] std::string_view StartString() const noexcept;
        Window& SetStartString(std::string_view content);

        Window& LoadString(std::string_view content);

        [[nodiscard]] std::string_view StartUrl() const noexcept;
        Window& SetStartUrl(std::string_view url);

        Window& Load(std::string_view url);

        Window& SendWebMessage(std::string_view message);

        // Getters

        [[nodiscard]] bool IsInitialized() const noexcept;
        [[nodiscard]] bool IsClosed() const noexcept;

        [[nodiscard]] Application& GetApplication() const noexcept;
        [[nodiscard]] Dispatcher& GetDispatcher() noexcept;
        [[nodiscard]] const Dispatcher& GetDispatcher() const noexcept;
        [[nodiscard]] Window* Parent() const noexcept;

        // Lifecycle methods

        void Show();
        void Close();

        Window& RegisterCreatingHandler(WindowHandler handler);
        [[nodiscard]] EventToken SubscribeCreatingHandler(WindowHandler handler);
        bool UnsubscribeCreatingHandler(EventToken token);

        Window& RegisterCreatedHandler(WindowHandler handler);
        [[nodiscard]] EventToken SubscribeCreatedHandler(WindowHandler handler);
        bool UnsubscribeCreatedHandler(EventToken token);

        Window& RegisterClosingHandler(ClosingHandler handler);
        [[nodiscard]] EventToken SubscribeClosingHandler(ClosingHandler handler);
        bool UnsubscribeClosingHandler(EventToken token);

        Window& RegisterClosedHandler(WindowHandler handler);
        [[nodiscard]] EventToken SubscribeClosedHandler(WindowHandler handler);
        bool UnsubscribeClosedHandler(EventToken token);

        Window& RegisterActivatedHandler(WindowHandler handler);
        [[nodiscard]] EventToken SubscribeActivatedHandler(WindowHandler handler);
        bool UnsubscribeActivatedHandler(EventToken token);

        Window& RegisterDeactivatedHandler(WindowHandler handler);
        [[nodiscard]] EventToken SubscribeDeactivatedHandler(WindowHandler handler);
        bool UnsubscribeDeactivatedHandler(EventToken token);

        Window& RegisterSizeChangedHandler(SizeChangedHandler handler);
        [[nodiscard]] EventToken SubscribeSizeChangedHandler(SizeChangedHandler handler);
        bool UnsubscribeSizeChangedHandler(EventToken token);

        Window& RegisterLocationChangedHandler(LocationChangedHandler handler);
        [[nodiscard]] EventToken SubscribeLocationChangedHandler(LocationChangedHandler handler);
        bool UnsubscribeLocationChangedHandler(EventToken token);

        Window& RegisterMaximizedHandler(WindowHandler handler);
        [[nodiscard]] EventToken SubscribeMaximizedHandler(WindowHandler handler);
        bool UnsubscribeMaximizedHandler(EventToken token);

        Window& RegisterRestoredHandler(WindowHandler handler);
        [[nodiscard]] EventToken SubscribeRestoredHandler(WindowHandler handler);
        bool UnsubscribeRestoredHandler(EventToken token);

        Window& RegisterMinimizedHandler(WindowHandler handler);
        [[nodiscard]] EventToken SubscribeMinimizedHandler(WindowHandler handler);
        bool UnsubscribeMinimizedHandler(EventToken token);

        Window& RegisterFullScreenEnteredHandler(WindowHandler handler);
        [[nodiscard]] EventToken SubscribeFullScreenEnteredHandler(WindowHandler handler);
        bool UnsubscribeFullScreenEnteredHandler(EventToken token);

        Window& RegisterFullScreenExitedHandler(WindowHandler handler);
        [[nodiscard]] EventToken SubscribeFullScreenExitedHandler(WindowHandler handler);
        bool UnsubscribeFullScreenExitedHandler(EventToken token);

        Window& RegisterStateChangedHandler(StateChangedHandler handler);
        [[nodiscard]] EventToken SubscribeStateChangedHandler(StateChangedHandler handler);
        bool UnsubscribeStateChangedHandler(EventToken token);

        Window& RegisterWebMessageReceivedHandler(WebMessageReceivedHandler handler);
        [[nodiscard]] EventToken SubscribeWebMessageReceivedHandler(WebMessageReceivedHandler handler);
        bool UnsubscribeWebMessageReceivedHandler(EventToken token);

        Window& RegisterNavigationStartingHandler(NavigationStartingHandler handler);
        [[nodiscard]] EventToken SubscribeNavigationStartingHandler(NavigationStartingHandler handler);
        bool UnsubscribeNavigationStartingHandler(EventToken token);

        Window& RegisterNewWindowRequestedHandler(NewWindowRequestedHandler handler);
        [[nodiscard]] EventToken SubscribeNewWindowRequestedHandler(NewWindowRequestedHandler handler);
        bool UnsubscribeNewWindowRequestedHandler(EventToken token);

        Window& RegisterContentLoadingHandler(ContentLoadingHandler handler);
        [[nodiscard]] EventToken SubscribeContentLoadingHandler(ContentLoadingHandler handler);
        bool UnsubscribeContentLoadingHandler(EventToken token);

        Window& RegisterContentLoadedHandler(ContentLoadedHandler handler);
        [[nodiscard]] EventToken SubscribeContentLoadedHandler(ContentLoadedHandler handler);
        bool UnsubscribeContentLoadedHandler(EventToken token);

    private:
        friend class Application;

        class Impl;
        std::unique_ptr<Impl> impl_;

        void InternalClose();
    };
}