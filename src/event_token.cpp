#include "event_token.internal.hpp"

#include <atomic>

namespace photinox
{
    std::uint64_t NextEventOwnerId() noexcept
    {
        static std::atomic_uint64_t lastOwnerId = 0;

        std::uint64_t ownerId;

        do
        {
            ownerId = lastOwnerId.fetch_add(1, std::memory_order_relaxed) + 1;
        }
        while (ownerId == 0);

        return ownerId;
    }
}