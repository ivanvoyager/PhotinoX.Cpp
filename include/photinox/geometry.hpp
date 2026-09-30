#pragma once

#include <photinox/enums.hpp>

#include <type_traits>

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
    static_assert(std::is_standard_layout_v<Rect>);
    static_assert(sizeof(Rect) == 16);

    struct Monitor
    {
        Rect monitor;
        Rect work;
        double scale = 1.0;

        friend constexpr bool operator==(Monitor, Monitor) noexcept = default;
    };
    static_assert(std::is_standard_layout_v<Monitor>);
    static_assert(sizeof(Monitor) == 40);

    struct Thickness
    {
        int left = 0;
        int top = 0;
        int right = 0;
        int bottom = 0;

        friend constexpr bool operator==(Thickness, Thickness) noexcept = default;
    };
    static_assert(std::is_standard_layout_v<Thickness>);
    static_assert(sizeof(Thickness) == 16);

    struct LayoutRegion
    {
        int width = 0;
        int height = 0;
        Thickness margin;
        HorizontalAlignment horizontalAlignment = HorizontalAlignment::Left;
        VerticalAlignment verticalAlignment = VerticalAlignment::Top;

        friend constexpr bool operator==(LayoutRegion, LayoutRegion) noexcept = default;
    };
    static_assert(std::is_standard_layout_v<LayoutRegion>);
    static_assert(sizeof(LayoutRegion) == 32);
}