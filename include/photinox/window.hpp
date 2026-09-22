#pragma once

#include <photinox/callbacks.hpp>
#include <photinox/event_token.hpp>
#include <photinox/geometry.hpp>

#include <memory>
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

        [[nodiscard]] std::string_view Title() const noexcept;
        Window& SetTitle(std::string_view title);

        [[nodiscard]] bool UseOsDefaultSize() const noexcept;
        Window& SetUseOsDefaultSize(bool useDefault);

        [[nodiscard]] bool UseOsDefaultLocation() const noexcept;
        Window& SetUseOsDefaultLocation(bool useDefault);

        [[nodiscard]] bool CenterOnInitialize() const noexcept;
        Window& SetCenterOnInitialize(bool center);

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

        // Browser

        Window& LoadString(std::string_view content);

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

    private:
        friend class Application;

        class Impl;
        std::unique_ptr<Impl> impl_;

        void InternalClose();
    };
}