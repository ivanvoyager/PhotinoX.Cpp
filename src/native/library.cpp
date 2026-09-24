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

            //memory
            freeString_ = LoadExport<decltype(freeString_)>("Photino_FreeString");

            //application
            applicationRun_ = LoadExport<decltype(applicationRun_)>("PhotinoApplication_Run");
            applicationShutdown_ = LoadExport<decltype(applicationShutdown_)>("PhotinoApplication_Shutdown");
            applicationIsRunning_ = LoadExport<decltype(applicationIsRunning_)>("PhotinoApplication_IsRunning");
            applicationIsShuttingDown_ = LoadExport<decltype(applicationIsShuttingDown_)>("PhotinoApplication_IsShuttingDown");
            applicationCheckAccess_ = LoadExport<decltype(applicationCheckAccess_)>("PhotinoApplication_CheckAccess");
            applicationInvoke_ = LoadExport<decltype(applicationInvoke_)>("PhotinoApplication_Invoke");
            applicationBeginInvoke_ = LoadExport<decltype(applicationBeginInvoke_)>("PhotinoApplication_BeginInvoke");

            //notifications
            applicationShowNotification_ = LoadExport<decltype(applicationShowNotification_)>("PhotinoApplication_ShowNotification");
            applicationGetNotificationsEnabled_ = LoadExport<decltype(applicationGetNotificationsEnabled_)>("PhotinoApplication_GetNotificationsEnabled");
            applicationSetNotificationsEnabled_ = LoadExport<decltype(applicationSetNotificationsEnabled_)>("PhotinoApplication_SetNotificationsEnabled");

            //window
            windowCreate_ = LoadExport<decltype(windowCreate_)>("Photino_ctor");
            windowShow_ = LoadExport<decltype(windowShow_)>("Photino_Show");
            windowCenter_ = LoadExport<decltype(windowCenter_)>("Photino_Center");
            windowActivate_ = LoadExport<decltype(windowActivate_)>("Photino_Activate");
            windowMaximize_ = LoadExport<decltype(windowMaximize_)>("Photino_Maximize");
            windowMinimize_ = LoadExport<decltype(windowMinimize_)>("Photino_Minimize");
            windowRestore_ = LoadExport<decltype(windowRestore_)>("Photino_Restore");
            windowClose_ = LoadExport<decltype(windowClose_)>("Photino_Close");

            windowGetTitle_ = LoadExport<decltype(windowGetTitle_)>("Photino_GetTitle");
            windowSetTitle_ = LoadExport<decltype(windowSetTitle_)>("Photino_SetTitle");

            windowGetIconFile_ = LoadExport<decltype(windowGetIconFile_)>("Photino_GetIconFile");
            windowSetIconFile_ = LoadExport<decltype(windowSetIconFile_)>("Photino_SetIconFile");

            windowGetPosition_ = LoadExport<decltype(windowGetPosition_)>("Photino_GetPosition");
            windowSetPosition_ = LoadExport<decltype(windowSetPosition_)>("Photino_SetPosition");

            windowGetSize_ = LoadExport<decltype(windowGetSize_)>("Photino_GetSize");
            windowSetSize_ = LoadExport<decltype(windowSetSize_)>("Photino_SetSize");

            windowSetMinSize_ = LoadExport<decltype(windowSetMinSize_)>("Photino_SetMinSize");
            windowSetMaxSize_ = LoadExport<decltype(windowSetMaxSize_)>("Photino_SetMaxSize");

            windowGetFullScreen_ = LoadExport<decltype(windowGetFullScreen_)>("Photino_GetFullScreen");
            windowSetFullScreen_ = LoadExport<decltype(windowSetFullScreen_)>("Photino_SetFullScreen");

            windowGetMaximized_ = LoadExport<decltype(windowGetMaximized_)>("Photino_GetMaximized");
            windowSetMaximized_ = LoadExport<decltype(windowSetMaximized_)>("Photino_SetMaximized");

            windowGetMinimized_ = LoadExport<decltype(windowGetMinimized_)>("Photino_GetMinimized");
            windowSetMinimized_ = LoadExport<decltype(windowSetMinimized_)>("Photino_SetMinimized");

            windowGetState_ = LoadExport<decltype(windowGetState_)>("Photino_GetWindowState");
            windowSetState_ = LoadExport<decltype(windowSetState_)>("Photino_SetWindowState");

            windowGetResizable_ = LoadExport<decltype(windowGetResizable_)>("Photino_GetResizable");
            windowSetResizable_ = LoadExport<decltype(windowSetResizable_)>("Photino_SetResizable");

            windowGetTopmost_ = LoadExport<decltype(windowGetTopmost_)>("Photino_GetTopmost");
            windowSetTopmost_ = LoadExport<decltype(windowSetTopmost_)>("Photino_SetTopmost");

            //browser
            windowNavigateToString_ = LoadExport<decltype(windowNavigateToString_)>("Photino_NavigateToString");
            windowSendWebMessage_ = LoadExport<decltype(windowSendWebMessage_)>("Photino_SendWebMessage");
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

    //memory

    void Library::FreeString(char* value) const noexcept
    {
        if (value)
            freeString_(value);
    }

    //application

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

    bool Library::ApplicationBeginInvoke(InvokeStateCallback callback, ReleaseStateCallback release, void* state) const noexcept
    {
        return applicationBeginInvoke_(callback, release, state);
    }

    //notifications

    int Library::ApplicationShowNotification(const NotificationShowParams* showParams) const
    {
        return applicationShowNotification_(showParams);
    }

    bool Library::ApplicationGetNotificationsEnabled() const noexcept
    {
        bool enabled = false;
        applicationGetNotificationsEnabled_(&enabled);
        return enabled;
    }

    void Library::ApplicationSetNotificationsEnabled(bool enabled) const noexcept
    {
        applicationSetNotificationsEnabled_(enabled);
    }

    //window

    void* Library::WindowCreate(WindowInitParams* initParams) const
    {
        return windowCreate_(initParams);
    }

    bool Library::WindowShow(void* instance) const
    {
        return windowShow_(instance);
    }

    bool Library::WindowCenter(void* instance) const noexcept
    {
        return windowCenter_(instance);
    }

    bool Library::WindowActivate(void* instance) const noexcept
    {
        return windowActivate_(instance);
    }

    bool Library::WindowMaximize(void* instance) const noexcept
    {
        return windowMaximize_(instance);
    }

    bool Library::WindowMinimize(void* instance) const noexcept
    {
        return windowMinimize_(instance);
    }

    bool Library::WindowRestore(void* instance) const noexcept
    {
        return windowRestore_(instance);
    }

    void Library::WindowClose(void* instance) const noexcept
    {
        windowClose_(instance);
    }

    std::string Library::WindowGetTitle(void* instance) const
    {
        char* value = windowGetTitle_(instance);

        if (!value)
            return {};

        try
        {
            std::string result(value);
            FreeString(value);
            return result;
        }
        catch (...)
        {
            FreeString(value);
            throw;
        }
    }

    void Library::WindowSetTitle(void* instance, const char* title) const noexcept
    {
        windowSetTitle_(instance, title);
    }

    std::string Library::WindowGetIconFile(void* instance) const
    {
        char* value = windowGetIconFile_(instance);

        if (!value)
            return {};

        try
        {
            std::string result(value);
            FreeString(value);
            return result;
        }
        catch (...)
        {
            FreeString(value);
            throw;
        }
    }

    void Library::WindowSetIconFile(void* instance, const char* iconFile) const noexcept
    {
        windowSetIconFile_(instance, iconFile);
    }

    Point Library::WindowGetPosition(void* instance) const noexcept
    {
        Point position;
        windowGetPosition_(instance, &position.x, &position.y);
        return position;
    }

    void Library::WindowSetPosition(void* instance, Point position) const noexcept
    {
        windowSetPosition_(instance, position.x, position.y);
    }

    Size Library::WindowGetSize(void* instance) const noexcept
    {
        Size size;
        windowGetSize_(instance, &size.width, &size.height);
        return size;
    }

    void Library::WindowSetSize(void* instance, Size size) const noexcept
    {
        windowSetSize_(instance, size.width, size.height);
    }

    void Library::WindowSetMinSize(void* instance, Size size) const noexcept
    {
        windowSetMinSize_(instance, size.width, size.height);
    }

    void Library::WindowSetMaxSize(void* instance, Size size) const noexcept
    {
        windowSetMaxSize_(instance, size.width, size.height);
    }

    bool Library::WindowGetFullScreen(void* instance) const noexcept
    {
        bool fullScreen = false;
        windowGetFullScreen_(instance, &fullScreen);
        return fullScreen;
    }

    void Library::WindowSetFullScreen(void* instance, bool fullScreen) const noexcept
    {
        windowSetFullScreen_(instance, fullScreen);
    }

    bool Library::WindowGetMaximized(void* instance) const noexcept
    {
        bool maximized = false;
        windowGetMaximized_(instance, &maximized);
        return maximized;
    }

    void Library::WindowSetMaximized(void* instance, bool maximized) const noexcept
    {
        windowSetMaximized_(instance, maximized);
    }

    bool Library::WindowGetMinimized(void* instance) const noexcept
    {
        bool minimized = false;
        windowGetMinimized_(instance, &minimized);
        return minimized;
    }

    void Library::WindowSetMinimized(void* instance, bool minimized) const noexcept
    {
        windowSetMinimized_(instance, minimized);
    }

    WindowState Library::WindowGetState(void* instance) const noexcept
    {
        WindowState state = WindowState::Normal;
        windowGetState_(instance, &state);
        return state;
    }

    void Library::WindowSetState(void* instance, WindowState state) const noexcept
    {
        windowSetState_(instance, state);
    }

    bool Library::WindowGetResizable(void* instance) const noexcept
    {
        bool resizable = false;
        windowGetResizable_(instance, &resizable);
        return resizable;
    }

    void Library::WindowSetResizable(void* instance, bool resizable) const noexcept
    {
        windowSetResizable_(instance, resizable);
    }

    bool Library::WindowGetTopmost(void* instance) const noexcept
    {
        bool topmost = false;
        windowGetTopmost_(instance, &topmost);
        return topmost;
    }

    void Library::WindowSetTopmost(void* instance, bool topmost) const noexcept
    {
        windowSetTopmost_(instance, topmost);
    }

    // browser

    void Library::WindowNavigateToString(void* instance, const char* content) const noexcept
    {
        windowNavigateToString_(instance, content);
    }

    void Library::WindowSendWebMessage(void* instance, const char* message) const noexcept
    {
        windowSendWebMessage_(instance, message);
    }
}