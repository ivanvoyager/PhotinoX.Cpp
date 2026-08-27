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
#else
#ifdef __APPLE__
        constexpr auto LibraryName = "@executable_path/PhotinoX.Native.dylib";
#else
        constexpr auto LibraryName = "$ORIGIN/PhotinoX.Native.so";
#endif
#endif
    }

    template<typename T>
    T Library::LoadExport(const char* name) const
    {
#ifdef _WIN32
        auto address = GetProcAddress(
            static_cast<HMODULE>(handle_),
            name);
#else
        auto address = dlsym(handle_, name);
#endif

        if (!address)
            throw std::runtime_error(
                std::string("Required PhotinoX.Native export was not found: ") + name);

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
            isApplicationRunning_ = LoadExport<decltype(isApplicationRunning_)>("PhotinoApplication_IsRunning");
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

    Library::~Library()
    {
        if (!handle_)
            return;

#ifdef _WIN32
        FreeLibrary(static_cast<HMODULE>(handle_));
#else
        dlclose(handle_);
#endif
    }

    const char* Library::GetVersion() const noexcept
    {
        return getVersion_();
    }

    bool Library::IsApplicationRunning() const noexcept
    {
        return isApplicationRunning_();
    }
}