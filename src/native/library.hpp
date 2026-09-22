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
        void WindowClose(void* instance) const noexcept;

        [[nodiscard]] std::string WindowGetTitle(void* instance) const;
        void WindowSetTitle(void* instance, const char* title) const noexcept;

        [[nodiscard]] Point WindowGetPosition(void* instance) const noexcept;
        void WindowSetPosition(void* instance, Point position) const noexcept;

        [[nodiscard]] Size WindowGetSize(void* instance) const noexcept;
        void WindowSetSize(void* instance, Size size) const noexcept;

        void WindowSetMinSize(void* instance, Size size) const noexcept;
        void WindowSetMaxSize(void* instance, Size size) const noexcept;

        //browser
        void WindowNavigateToString(void* instance, const char* content) const noexcept;

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
        void (*windowClose_)(void*) = nullptr;

        char* (*windowGetTitle_)(void*) = nullptr;
        void (*windowSetTitle_)(void*, const char*) = nullptr;

        void (*windowGetPosition_)(void*, int*, int*) = nullptr;
        void (*windowSetPosition_)(void*, int, int) = nullptr;

        void (*windowGetSize_)(void*, int*, int*) = nullptr;
        void (*windowSetSize_)(void*, int, int) = nullptr;

        void (*windowSetMinSize_)(void*, int, int) = nullptr;
        void (*windowSetMaxSize_)(void*, int, int) = nullptr;

        //browser
        void (*windowNavigateToString_)(void*, const char*) = nullptr;
    };
}