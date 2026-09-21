#pragma once

#include <photinox/enums.hpp>

#include <cstddef>
#include <type_traits>

namespace photinox::native
{
    using Utf8String = const char*;

    using CreatedCallback = void (*)(void* instance, bool registered, void* state);
    using ClosingCallback = bool (*)(void* state);
    using ClosedCallback = void (*)(void* state);
    using FocusInCallback = void (*)(void* state);
    using FocusOutCallback = void (*)(void* state);
    using ResizedCallback = void (*)(int width, int height, void* state);
    using MovedCallback = void (*)(int x, int y, void* state);
    using MaximizedCallback = void (*)(void* state);
    using RestoredCallback = void (*)(void* state);
    using MinimizedCallback = void (*)(void* state);
    using FullScreenChangedCallback = void (*)(bool fullScreen, void* state);
    using StateChangedCallback = void (*)(WindowState oldState, WindowState newState, void* state);
    using WebMessageReceivedCallback = void (*)(Utf8String message, Utf8String uri, void* state);
    using CustomSchemeCallback = void* (*)(Utf8String url, int* numBytes, Utf8String* contentType, void* state);
    using NavigationStartingCallback = bool (*)(Utf8String uri, void* state);
    using NewWindowRequestedCallback = bool (*)(Utf8String uri, void* state);
    using ContentLoadingCallback = void (*)(Utf8String uri, void* state);
    using ContentLoadedCallback = void (*)(Utf8String uri, void* state);

    struct WindowInitCallbacks
    {
        CreatedCallback createdHandler;
        ClosingCallback closingHandler;
        ClosedCallback closedHandler;
        FocusInCallback focusInHandler;
        FocusOutCallback focusOutHandler;
        ResizedCallback resizedHandler;
        MovedCallback movedHandler;
        MaximizedCallback maximizedHandler;
        RestoredCallback restoredHandler;
        MinimizedCallback minimizedHandler;
        FullScreenChangedCallback fullScreenChangedHandler;
        StateChangedCallback stateChangedHandler;
        WebMessageReceivedCallback webMessageReceivedHandler;
        CustomSchemeCallback customSchemeHandler;
        NavigationStartingCallback navigationStartingHandler;
        NewWindowRequestedCallback newWindowRequestedHandler;
        ContentLoadingCallback contentLoadingHandler;
        ContentLoadedCallback contentLoadedHandler;

        void* callbackState;
    };

    static_assert(std::is_standard_layout_v<WindowInitCallbacks>);
    static_assert(sizeof(WindowInitCallbacks) == 152);

    struct WindowInitOptions
    {
        Utf8String title;
        Utf8String iconFile;

        bool chromeless;
        bool transparent;
        bool useNativeWindowOwner;
    };

    static_assert(std::is_standard_layout_v<WindowInitOptions>);
    static_assert(sizeof(WindowInitOptions) == 24);

    struct WindowInitLinuxChromelessOptions
    {
        int dragRegionHeight;
        int dragRegionLeftInset;
        int dragRegionTopInset;
        int dragRegionRightInset;
        int resizeBorderThickness;
    };

    static_assert(std::is_standard_layout_v<WindowInitLinuxChromelessOptions>);
    static_assert(sizeof(WindowInitLinuxChromelessOptions) == 20);

    struct WindowInitGeometry
    {
        int left;
        int top;
        int width;
        int height;
        int minWidth;
        int minHeight;
        int maxWidth;
        int maxHeight;

        WindowState windowState;

        bool centerOnInitialize;
        bool resizable;
        bool topmost;
        bool useOsDefaultLocation;
        bool useOsDefaultSize;
    };

    static_assert(std::is_standard_layout_v<WindowInitGeometry>);
    static_assert(sizeof(WindowInitGeometry) == 44);

    inline constexpr int MaxCustomSchemeNames = 16;

    struct WindowInitBrowserOptions
    {
        Utf8String startString;
        Utf8String startUrl;

        Utf8String userDataFolder;
        Utf8String userAgent;
        Utf8String controlInitParameters;
        Utf8String customSchemeNames[MaxCustomSchemeNames];

        int zoom;
        bool zoomEnabled;
        bool contextMenuEnabled;
        bool statusBarEnabled;
        bool devToolsEnabled;

        bool grantBrowserPermissions;
        bool mediaAutoplayEnabled;
        bool fileSystemAccessEnabled;
        bool webSecurityEnabled;
        bool javascriptClipboardAccessEnabled;
        bool mediaStreamEnabled;
        bool smoothScrollingEnabled;
        bool ignoreCertificateErrorsEnabled;
    };

    static_assert(std::is_standard_layout_v<WindowInitBrowserOptions>);
    static_assert(sizeof(WindowInitBrowserOptions) == 184);

    struct WindowInitParams
    {
        static constexpr int NativeAbiVersion = 7;

        int size;
        int abiVersion;

        void* parentInstance;

        WindowInitCallbacks callbacks;
        WindowInitOptions window;
        WindowInitLinuxChromelessOptions linuxChromeless;
        WindowInitGeometry geometry;
        WindowInitBrowserOptions browser;
    };

    static_assert(std::is_standard_layout_v<WindowInitParams>);

    static_assert(offsetof(WindowInitParams, callbacks) == 16);
    static_assert(offsetof(WindowInitParams, window) == 168);
    static_assert(offsetof(WindowInitParams, linuxChromeless) == 192);
    static_assert(offsetof(WindowInitParams, geometry) == 212);
    static_assert(offsetof(WindowInitParams, browser) == 256);

    static_assert(sizeof(WindowInitParams) == 440);
}