#include "library.hpp"

#include <stdexcept>

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

    Library::Library()
    {
#ifdef _WIN32
        handle_ = LoadLibraryW(LibraryName);

        if (!handle_)
            throw std::runtime_error("Failed to load PhotinoX.Native.dll.");

        getVersion_ = reinterpret_cast<const char* (*)()>(
            GetProcAddress(
                static_cast<HMODULE>(handle_),
                "Photino_GetNativeVersion"));
#else
        handle_ = dlopen(LibraryName, RTLD_NOW | RTLD_LOCAL);

        if (!handle_)
            throw std::runtime_error(dlerror());

        getVersion_ = reinterpret_cast<const char* (*)()>(
            dlsym(handle_, "Photino_GetNativeVersion"));
#endif

        if (!getVersion_)
        {
#ifdef _WIN32
            FreeLibrary(static_cast<HMODULE>(handle_));
#else
            dlclose(handle_);
#endif
            handle_ = nullptr;

            throw std::runtime_error(
                "Photino_GetNativeVersion was not found.");
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
}