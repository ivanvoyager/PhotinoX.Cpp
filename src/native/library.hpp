#pragma once

#include <photinox/geometry.hpp>

#include "callbacks.hpp"
#include "application.hpp"
#include "notification.hpp"
#include "window.hpp"

#include <string>

namespace photinox::native
{
    class Library final
    {
    public:
        Library();
        ~Library();

        Library(const Library&) = delete;
        Library& operator=(const Library&) = delete;

        Library(Library&&) = delete;
        Library& operator=(Library&&) = delete;

        [[nodiscard]] const char* GetVersion() const noexcept;

        //memory
        void FreeString(char* value) const noexcept;

        //application
        [[nodiscard]] int ApplicationRun(const ApplicationInitParams* initParams) const;
        void ApplicationShutdown(int exitCode, bool force) const noexcept;
        [[nodiscard]] bool ApplicationIsRunning() const noexcept;
        [[nodiscard]] bool ApplicationIsShuttingDown() const noexcept;
        [[nodiscard]] bool ApplicationCheckAccess() const noexcept;
        [[nodiscard]] bool ApplicationInvoke(InvokeStateCallback callback, void* state) const;
        [[nodiscard]] bool ApplicationBeginInvoke(InvokeStateCallback callback, ReleaseStateCallback release, void* state) const noexcept;
        //notifications
        [[nodiscard]] int ApplicationShowNotification(const NotificationShowParams* showParams) const;
        [[nodiscard]] bool ApplicationGetNotificationsEnabled() const noexcept;
        void ApplicationSetNotificationsEnabled(bool enabled) const noexcept;
        //window
        [[nodiscard]] void* WindowCreate(WindowInitParams* initParams) const;
        [[nodiscard]] bool WindowShow(void* instance) const;
        [[nodiscard]] bool WindowCenter(void* instance) const noexcept;
        [[nodiscard]] bool WindowActivate(void* instance) const noexcept;
        [[nodiscard]] bool WindowMaximize(void* instance) const noexcept;
        [[nodiscard]] bool WindowMinimize(void* instance) const noexcept;
        [[nodiscard]] bool WindowRestore(void* instance) const noexcept;
        void WindowClose(void* instance) const noexcept;

        [[nodiscard]] std::string WindowGetTitle(void* instance) const;
        void WindowSetTitle(void* instance, const char* title) const noexcept;

        [[nodiscard]] std::string WindowGetIconFile(void* instance) const;
        void WindowSetIconFile(void* instance, const char* iconFile) const noexcept;

        [[nodiscard]] Point WindowGetPosition(void* instance) const noexcept;
        void WindowSetPosition(void* instance, Point position) const noexcept;

        [[nodiscard]] Size WindowGetSize(void* instance) const noexcept;
        void WindowSetSize(void* instance, Size size) const noexcept;

        void WindowSetMinSize(void* instance, Size size) const noexcept;
        void WindowSetMaxSize(void* instance, Size size) const noexcept;

        [[nodiscard]] bool WindowGetFullScreen(void* instance) const noexcept;
        void WindowSetFullScreen(void* instance, bool fullScreen) const noexcept;

        [[nodiscard]] bool WindowGetMaximized(void* instance) const noexcept;
        void WindowSetMaximized(void* instance, bool maximized) const noexcept;

        [[nodiscard]] bool WindowGetMinimized(void* instance) const noexcept;
        void WindowSetMinimized(void* instance, bool minimized) const noexcept;

        [[nodiscard]] WindowState WindowGetState(void* instance) const noexcept;
        void WindowSetState(void* instance, WindowState state) const noexcept;

        [[nodiscard]] bool WindowGetResizable(void* instance) const noexcept;
        void WindowSetResizable(void* instance, bool resizable) const noexcept;

        [[nodiscard]] bool WindowGetTopmost(void* instance) const noexcept;
        void WindowSetTopmost(void* instance, bool topmost) const noexcept;

        //browser
        void WindowNavigateToString(void* instance, const char* content) const noexcept;
        void WindowNavigateToUrl(void* instance, const char* url) const noexcept;
        void WindowSendWebMessage(void* instance, const char* message) const noexcept;

        [[nodiscard]] bool WindowGetContextMenuEnabled(void* instance) const noexcept;
        void WindowSetContextMenuEnabled(void* instance, bool enabled) const noexcept;

        [[nodiscard]] bool WindowGetZoomEnabled(void* instance) const noexcept;
        void WindowSetZoomEnabled(void* instance, bool enabled) const noexcept;

        [[nodiscard]] bool WindowGetStatusBarEnabled(void* instance) const noexcept;
        void WindowSetStatusBarEnabled(void* instance, bool enabled) const noexcept;

        [[nodiscard]] bool WindowGetDevToolsEnabled(void* instance) const noexcept;
        void WindowSetDevToolsEnabled(void* instance, bool enabled) const noexcept;

        [[nodiscard]] int WindowGetZoom(void* instance) const noexcept;
        void WindowSetZoom(void* instance, int zoom) const noexcept;

        [[nodiscard]] bool WindowGetGrantBrowserPermissions(void* instance) const noexcept;
        [[nodiscard]] bool WindowGetMediaAutoplayEnabled(void* instance) const noexcept;
        [[nodiscard]] bool WindowGetFileSystemAccessEnabled(void* instance) const noexcept;
        [[nodiscard]] bool WindowGetWebSecurityEnabled(void* instance) const noexcept;
        [[nodiscard]] bool WindowGetJavascriptClipboardAccessEnabled(void* instance) const noexcept;
        [[nodiscard]] bool WindowGetMediaStreamEnabled(void* instance) const noexcept;
        [[nodiscard]] bool WindowGetSmoothScrollingEnabled(void* instance) const noexcept;
        [[nodiscard]] bool WindowGetIgnoreCertificateErrorsEnabled(void* instance) const noexcept;

        [[nodiscard]] bool WindowGetTransparentEnabled(void* instance) const noexcept;
        void WindowSetTransparentEnabled(void* instance, bool enabled) const noexcept;

        void WindowClearBrowserAutoFill(void* instance) const noexcept;

    private:
        template<typename T>
        [[nodiscard]] T LoadExport(const char* name) const;

        void* handle_ = nullptr;

        const char* (*getVersion_)() = nullptr;

        //memory
        void (*freeString_)(char*) = nullptr;

        //application
        int (*applicationRun_)(const ApplicationInitParams*) = nullptr;
        void (*applicationShutdown_)(int, bool) = nullptr;
        bool (*applicationIsRunning_)() = nullptr;
        bool (*applicationIsShuttingDown_)() = nullptr;
        bool (*applicationCheckAccess_)() = nullptr;
        bool (*applicationInvoke_)(InvokeStateCallback, void*) = nullptr;
        bool (*applicationBeginInvoke_)(InvokeStateCallback, ReleaseStateCallback, void*) = nullptr;
        //notifications
        int (*applicationShowNotification_)(const NotificationShowParams*) = nullptr;
        void (*applicationGetNotificationsEnabled_)(bool*) = nullptr;
        void (*applicationSetNotificationsEnabled_)(bool) = nullptr;
        //window
        void* (*windowCreate_)(WindowInitParams*) = nullptr;
        bool (*windowShow_)(void*) = nullptr;
        bool (*windowCenter_)(void*) = nullptr;
        bool (*windowActivate_)(void*) = nullptr;
        bool (*windowMaximize_)(void*) = nullptr;
        bool (*windowMinimize_)(void*) = nullptr;
        bool (*windowRestore_)(void*) = nullptr;
        void (*windowClose_)(void*) = nullptr;

        char* (*windowGetTitle_)(void*) = nullptr;
        void (*windowSetTitle_)(void*, const char*) = nullptr;

        char* (*windowGetIconFile_)(void*) = nullptr;
        void (*windowSetIconFile_)(void*, const char*) = nullptr;

        void (*windowGetPosition_)(void*, int*, int*) = nullptr;
        void (*windowSetPosition_)(void*, int, int) = nullptr;

        void (*windowGetSize_)(void*, int*, int*) = nullptr;
        void (*windowSetSize_)(void*, int, int) = nullptr;

        void (*windowSetMinSize_)(void*, int, int) = nullptr;
        void (*windowSetMaxSize_)(void*, int, int) = nullptr;

        void (*windowGetFullScreen_)(void*, bool*) = nullptr;
        void (*windowSetFullScreen_)(void*, bool) = nullptr;

        void (*windowGetMaximized_)(void*, bool*) = nullptr;
        void (*windowSetMaximized_)(void*, bool) = nullptr;

        void (*windowGetMinimized_)(void*, bool*) = nullptr;
        void (*windowSetMinimized_)(void*, bool) = nullptr;

        void (*windowGetState_)(void*, WindowState*) = nullptr;
        void (*windowSetState_)(void*, WindowState) = nullptr;

        void (*windowGetResizable_)(void*, bool*) = nullptr;
        void (*windowSetResizable_)(void*, bool) = nullptr;

        void (*windowGetTopmost_)(void*, bool*) = nullptr;
        void (*windowSetTopmost_)(void*, bool) = nullptr;

        //browser
        void (*windowNavigateToString_)(void*, const char*) = nullptr;
        void (*windowNavigateToUrl_)(void*, const char*) = nullptr;
        void (*windowSendWebMessage_)(void*, const char*) = nullptr;

        void (*windowGetContextMenuEnabled_)(void*, bool*) = nullptr;
        void (*windowSetContextMenuEnabled_)(void*, bool) = nullptr;

        void (*windowGetZoomEnabled_)(void*, bool*) = nullptr;
        void (*windowSetZoomEnabled_)(void*, bool) = nullptr;

        void (*windowGetStatusBarEnabled_)(void*, bool*) = nullptr;
        void (*windowSetStatusBarEnabled_)(void*, bool) = nullptr;

        void (*windowGetDevToolsEnabled_)(void*, bool*) = nullptr;
        void (*windowSetDevToolsEnabled_)(void*, bool) = nullptr;

        void (*windowGetZoom_)(void*, int*) = nullptr;
        void (*windowSetZoom_)(void*, int) = nullptr;

        void (*windowGetGrantBrowserPermissions_)(void*, bool*) = nullptr;
        void (*windowGetMediaAutoplayEnabled_)(void*, bool*) = nullptr;
        void (*windowGetFileSystemAccessEnabled_)(void*, bool*) = nullptr;
        void (*windowGetWebSecurityEnabled_)(void*, bool*) = nullptr;
        void (*windowGetJavascriptClipboardAccessEnabled_)(void*, bool*) = nullptr;
        void (*windowGetMediaStreamEnabled_)(void*, bool*) = nullptr;
        void (*windowGetSmoothScrollingEnabled_)(void*, bool*) = nullptr;
        void (*windowGetIgnoreCertificateErrorsEnabled_)(void*, bool*) = nullptr;

        void (*windowGetTransparentEnabled_)(void*, bool*) = nullptr;
        void (*windowSetTransparentEnabled_)(void*, bool) = nullptr;

        void (*windowClearBrowserAutoFill_)(void*) = nullptr;
    };
}