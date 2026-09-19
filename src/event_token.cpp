#include "event_token.internal.hpp"

#include <atomic>
#include <stdexcept>

namespace photinox
{
    std::uint64_t NextEventOwnerId()
    {
        static std::atomic_uint64_t nextOwnerId = 1;

        const std::uint64_t ownerId = nextOwnerId.fetch_add(1, std::memory_order_relaxed);

        if (ownerId == 0)
            throw std::overflow_error("Event owner identifier limit has been reached.");

        return ownerId;
    }
}