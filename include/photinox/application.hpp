#pragma once

#include <memory>
#include <string_view>

namespace photinox
{
    class Application final
    {
    public:
        Application();
        ~Application();

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        Application(Application&&) noexcept;
        Application& operator=(Application&&) noexcept;

        [[nodiscard]] std::string_view NativeVersion() const noexcept;

    private:
        class Impl;
        std::unique_ptr<Impl> impl_;
    };
}