#pragma once

#include <cstdint>

namespace photinox
{
    class Dispatcher;

    class EventToken final
    {
    public:
        constexpr EventToken() noexcept = default;

        [[nodiscard]] constexpr explicit operator bool() const noexcept
        {
            return value_ != 0;
        }

        friend constexpr bool operator==(EventToken, EventToken) noexcept = default;

    private:
        friend class Application;
        friend class Dispatcher;

        constexpr explicit EventToken(std::uint64_t value) noexcept
            : value_(value)
        {
        }

        std::uint64_t value_ = 0;
    };
}