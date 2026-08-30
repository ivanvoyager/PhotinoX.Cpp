#include "library.hpp"

#include <stdexcept>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace photinox::native
{
    namespace
    {
#ifdef _WIN32
        constexpr auto LibraryName = L"PhotinoX.Native.dll";
#elif defined(__APPLE__)
        constexpr auto LibraryName = "@executable_path/PhotinoX.Native.dylib";
#else
        constexpr auto LibraryName = "$ORIGIN/PhotinoX.Native.so";
#endif
    }

    template<typename T>
    T Library::LoadExport(const char* name) const
    {
#ifdef _WIN32
        auto address = GetProcAddress(static_cast<HMODULE>(handle_), name);
#else
        auto address = dlsym(handle_, name);
#endif

        if (!address)
            throw std::runtime_error(std::string("Required PhotinoX.Native export was not found: ") + name);

        return reinterpret_cast<T>(address);
    }

    Library::Library()
    {
#ifdef _WIN32
        handle_ = LoadLibraryW(LibraryName);

        if (!handle_)
            throw std::runtime_error("Failed to load PhotinoX.Native.dll.");
#else
        handle_ = dlopen(LibraryName, RTLD_NOW | RTLD_LOCAL);

        if (!handle_)
            throw std::runtime_error(dlerror());
#endif

        try
        {
            getVersion_ = LoadExport<decltype(getVersion_)>("Photino_GetNativeVersion");

            applicationRun_ = LoadExport<decltype(applicationRun_)>("PhotinoApplication_Run");
            applicationShutdown_ = LoadExport<decltype(applicationShutdown_)>("PhotinoApplication_Shutdown");
            applicationIsRunning_ = LoadExport<decltype(applicationIsRunning_)>("PhotinoApplication_IsRunning");
            applicationIsShuttingDown_ = LoadExport<decltype(applicationIsShuttingDown_)>("PhotinoApplication_IsShuttingDown");

            applicationCheckAccess_ = LoadExport<decltype(applicationCheckAccess_)>("PhotinoApplication_CheckAccess");
            applicationInvoke_ = LoadExport<decltype(applicationInvoke_)>("PhotinoApplication_Invoke");
            applicationBeginInvoke_ = LoadExport<decltype(applicationBeginInvoke_)>("PhotinoApplication_BeginInvoke");
        }
        catch (...)
        {
#ifdef _WIN32
            FreeLibrary(static_cast<HMODULE>(handle_));
#else
            dlclose(handle_);
#endif

            handle_ = nullptr;
            throw;
        }
    }

    Library::~Library() = default;

    const char* Library::GetVersion() const noexcept
    {
        return getVersion_();
    }

    int Library::ApplicationRun(const ApplicationInitParams* initParams) const
    {
        return applicationRun_(initParams);
    }

    void Library::ApplicationShutdown(int exitCode, bool force) const noexcept
    {
        applicationShutdown_(exitCode, force);
    }

    bool Library::ApplicationIsRunning() const noexcept
    {
        return applicationIsRunning_();
    }

    bool Library::ApplicationIsShuttingDown() const noexcept
    {
        return applicationIsShuttingDown_();
    }

    bool Library::ApplicationCheckAccess() const noexcept
    {
        return applicationCheckAccess_();
    }

    bool Library::ApplicationInvoke(InvokeStateCallback callback, void* state) const
    {
        return applicationInvoke_(callback, state);
    }

    bool Library::ApplicationBeginInvoke(InvokeStateCallback callback, void* state) const
    {
        return applicationBeginInvoke_(callback, state);
    }
}