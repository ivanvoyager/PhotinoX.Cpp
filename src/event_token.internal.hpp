#pragma once

#include <cstdint>

namespace photinox::detail
{
    [[nodiscard]] std::uint64_t NextEventOwnerId() noexcept;
}