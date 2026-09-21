#pragma once

#include <cstddef>
#include <type_traits>

namespace photinox::native
{
    struct NotificationShowParams
    {
        static constexpr int NativeAbiVersion = 1;

        int size;
        int abiVersion;

        int notificationId;

        const char* title;
        const char* body;
        const char* iconPath;

        void* callbackState;
    };

    static_assert(std::is_standard_layout_v<NotificationShowParams>);
    static_assert(sizeof(NotificationShowParams) == 48);
}