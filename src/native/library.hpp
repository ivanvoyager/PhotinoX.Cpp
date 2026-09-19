#pragma once

#include <photinox/callbacks.hpp>

#include "application.hpp"
#include "window.hpp"

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

        //window
        [[nodiscard]] void* WindowCreate(WindowInitParams* initParams) const;
        [[nodiscard]] bool WindowShow(void* instance) const;
        void WindowClose(void* instance) const noexcept;

        //application
        [[nodiscard]] int ApplicationRun(const ApplicationInitParams* initParams) const;
        void ApplicationShutdown(int exitCode, bool force) const noexcept;
        [[nodiscard]] bool ApplicationIsRunning() const noexcept;
        [[nodiscard]] bool ApplicationIsShuttingDown() const noexcept;
        [[nodiscard]] bool ApplicationCheckAccess() const noexcept;
        [[nodiscard]] bool ApplicationInvoke(InvokeStateCallback callback, void* state) const;
        [[nodiscard]] bool ApplicationBeginInvoke(InvokeStateCallback callback, ReleaseStateCallback release, void* state) const noexcept;
        //notifications
        [[nodiscard]] bool ApplicationGetNotificationsEnabled() const noexcept;
        void ApplicationSetNotificationsEnabled(bool enabled) const noexcept;

    private:
        template<typename T>
        [[nodiscard]] T LoadExport(const char* name) const;

        void* handle_ = nullptr;

        const char* (*getVersion_)() = nullptr;

        //window
        void* (*windowCreate_)(WindowInitParams*) = nullptr;
        bool (*windowShow_)(void*) = nullptr;
        void (*windowClose_)(void*) = nullptr;

        //application
        int (*applicationRun_)(const ApplicationInitParams*) = nullptr;
        void (*applicationShutdown_)(int, bool) = nullptr;
        bool (*applicationIsRunning_)() = nullptr;
        bool (*applicationIsShuttingDown_)() = nullptr;
        bool (*applicationCheckAccess_)() = nullptr;
        bool (*applicationInvoke_)(InvokeStateCallback, void*) = nullptr;
        bool (*applicationBeginInvoke_)(InvokeStateCallback, ReleaseStateCallback, void*) = nullptr;
        //notifications
        void (*applicationGetNotificationsEnabled_)(bool*) = nullptr;
        void (*applicationSetNotificationsEnabled_)(bool) = nullptr;
    };
}