#pragma once

namespace photinox
{
    class Platform final
    {
    public:
#if defined(_WIN32)
        static constexpr bool IsWindows = true;
#else
        static constexpr bool IsWindows = false;
#endif

#if defined(__linux__)
        static constexpr bool IsLinux = true;
#else
        static constexpr bool IsLinux = false;
#endif

#if defined(__APPLE__)
        static constexpr bool IsMacOS = true;
#else
        static constexpr bool IsMacOS = false;
#endif

        Platform() = delete;
    };

    static_assert(
        static_cast<int>(Platform::IsWindows) +
        static_cast<int>(Platform::IsLinux) +
        static_cast<int>(Platform::IsMacOS) == 1,
        "Unsupported or ambiguous target platform.");
}