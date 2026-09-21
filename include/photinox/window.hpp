#pragma once

#include <photinox/callbacks.hpp>
#include <photinox/event_token.hpp>

#include <memory>
#include <string_view>

namespace photinox
{
    class Application;

    class Window final
    {
    public:
        explicit Window(Application& application, Window* parent = nullptr);
        ~Window();

        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;

        Window(Window&&) = delete;
        Window& operator=(Window&&) = delete;

        [[nodiscard]] std::string_view Title() const noexcept;
        Window& SetTitle(std::string_view title);

        [[nodiscard]] bool IsInitialized() const noexcept;
        [[nodiscard]] bool IsClosed() const noexcept;

        [[nodiscard]] Application& GetApplication() const noexcept;
        [[nodiscard]] Window* Parent() const noexcept;

        Window& LoadString(std::string_view content);

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