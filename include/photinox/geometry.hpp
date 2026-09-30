#pragma once

namespace photinox
{
    struct Point
    {
        int x = 0;
        int y = 0;

        friend constexpr bool operator==(Point, Point) noexcept = default;
    };

    struct Size
    {
        int width = 0;
        int height = 0;

        friend constexpr bool operator==(Size, Size) noexcept = default;
    };

    struct Rect
    {
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;

        friend constexpr bool operator==(Rect, Rect) noexcept = default;
    };

    struct Monitor
    {
        Rect monitor;
        Rect work;
        double scale = 1.0;

        friend constexpr bool operator==(Monitor, Monitor) noexcept = default;
    };
}