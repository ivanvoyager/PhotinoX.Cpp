#pragma once

#include <cstdint>

namespace photinox
{
    class EventSubscriptionRegistry;

    class EventToken final
    {
    public:
        constexpr EventToken() noexcept = default;

        [[nodiscard]] constexpr explicit operator bool() const noexcept
        {
            return ownerId_ != 0 && value_ != 0;
        }

        friend constexpr bool operator==(EventToken, EventToken) noexcept = default;

    private:
        friend class EventSubscriptionRegistry;

        constexpr EventToken(std::uint64_t ownerId, std::uint64_t value) noexcept
            : ownerId_(ownerId),
            value_(value)
        {
        }

        std::uint64_t ownerId_ = 0;
        std::uint64_t value_ = 0;
    };
}