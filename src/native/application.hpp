#pragma once

#include <photinox/enums.hpp>

#include <cstddef>
#include <type_traits>

namespace photinox::native
{
    using StartupCallback = void (*)(void* state);
    using ShutdownRequestedCallback = bool (*)(ShutdownRequestReason reason, void* state);
    using ExitCallback = int (*)(int exitCode, void* state);

    using WindowCollectionChangedCallback = void (*)(NotifyCollectionChangedAction action, void* const* newItems, int newItemsCount, void* const* oldItems, int oldItemsCount, void* state);

    using NotificationActivatedCallback = void (*)(int notificationId, void* notificationState, void* state);
    using NotificationActionActivatedCallback = void (*)(int notificationId, int actionIndex, void* notificationState, void* state);
    using NotificationInputActivatedCallback = void (*)(int notificationId, const char* response, void* notificationState, void* state);
    using NotificationDismissedCallback = void (*)(int notificationId, NotificationDismissalReason reason, void* notificationState, void* state);
    using NotificationFailedCallback = void (*)(int notificationId, void* notificationState, void* state);

    struct ApplicationInitCallbacks
    {
        StartupCallback startupHandler;
        ShutdownRequestedCallback shutdownRequestedHandler;
        ExitCallback exitHandler;
        WindowCollectionChangedCallback windowCollectionChangedHandler;
        void* callbackState;
    };

    static_assert(std::is_standard_layout_v<ApplicationInitCallbacks>);
    static_assert(sizeof(ApplicationInitCallbacks) == 40);

    struct ApplicationInitOptions
    {
        const char* applicationName;
        const char* applicationIconPath;
        const char* notificationRegistrationId;

        bool notificationsEnabled;
    };

    static_assert(std::is_standard_layout_v<ApplicationInitOptions>);
    static_assert(sizeof(ApplicationInitOptions) == 32);

    struct NotificationCallbacks
    {
        NotificationActivatedCallback notificationActivatedHandler;
        NotificationActionActivatedCallback notificationActionActivatedHandler;
        NotificationInputActivatedCallback notificationInputActivatedHandler;
        NotificationDismissedCallback notificationDismissedHandler;
        NotificationFailedCallback notificationFailedHandler;
    };

    static_assert(std::is_standard_layout_v<NotificationCallbacks>);
    static_assert(sizeof(NotificationCallbacks) == 40);

    struct ApplicationInitParams
    {
        static constexpr int NativeAbiVersion = 3;

        int size;                                       // #1
        int abiVersion;                                 // #2

        ApplicationInitCallbacks callbacks;             // #3
        ApplicationInitOptions options;                 // #4
        NotificationCallbacks notificationCallbacks;    // #5
    };

    static_assert(std::is_standard_layout_v<ApplicationInitParams>);

    static_assert(offsetof(ApplicationInitParams, callbacks) == 8);
    static_assert(offsetof(ApplicationInitParams, options) == 48);
    static_assert(offsetof(ApplicationInitParams, notificationCallbacks) == 80);

    static_assert(sizeof(ApplicationInitParams) == 120);
}