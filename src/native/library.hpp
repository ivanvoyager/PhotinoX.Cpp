#pragma once

#include <photinox/callbacks.hpp>

#include "application.hpp"

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

        [[nodiscard]] int ApplicationRun(const ApplicationInitParams* initParams) const;
        void ApplicationShutdown(int exitCode, bool force) const noexcept;
        [[nodiscard]] bool ApplicationIsRunning() const noexcept;
        [[nodiscard]] bool ApplicationIsShuttingDown() const noexcept;

        [[nodiscard]] bool ApplicationCheckAccess() const noexcept;
        [[nodiscard]] bool ApplicationInvoke(InvokeStateCallback callback, void* state) const;
        [[nodiscard]] bool ApplicationBeginInvoke(InvokeStateCallback callback, void* state) const;

    private:
        template<typename T>
        [[nodiscard]] T LoadExport(const char* name) const;

        void* handle_ = nullptr;

        const char* (*getVersion_)() = nullptr;

        int (*applicationRun_)(const ApplicationInitParams*) = nullptr;
        void (*applicationShutdown_)(int, bool) = nullptr;
        bool (*applicationIsRunning_)() = nullptr;
        bool (*applicationIsShuttingDown_)() = nullptr;

        bool (*applicationCheckAccess_)() = nullptr;
        bool (*applicationInvoke_)(InvokeStateCallback, void*) = nullptr;
        bool (*applicationBeginInvoke_)(InvokeStateCallback, void*) = nullptr;
    };
}