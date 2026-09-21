#pragma once

#include <photinox/callbacks.hpp>

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

        [[nodiscard]] bool IsInitialized() const noexcept;
        [[nodiscard]] bool IsClosed() const noexcept;

        [[nodiscard]] Application& GetApplication() const noexcept;
        [[nodiscard]] Window* Parent() const noexcept;

        Window& SetTitle(std::string_view title);
        Window& LoadString(std::string_view content);

        Window& RegisterCreatingHandler(WindowHandler handler);
        Window& RegisterCreatedHandler(WindowHandler handler);
        Window& RegisterClosingHandler(ClosingHandler handler);
        Window& RegisterClosedHandler(WindowHandler handler);

        void Show();
        void Close();

    private:
        friend class Application;

        class Impl;
        std::unique_ptr<Impl> impl_;

        void InternalClose();
    };
}