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

    namespace
    {
        std::vector<std::string> CopyStringArray(char** values, int count)
        {
            std::vector<std::string> result;

            if (!values || count <= 0)
                return result;

            result.reserve(static_cast<std::size_t>(count));

            for (int index = 0; index < count; ++index)
                result.emplace_back(values[index] ? values[index] : "");

            return result;
        }
    }

    template<typename T>
    T Library::LoadExport(const char* name) const
    {
#ifdef _WIN32
        auto address = GetProcAddress(static_cast<HMODULE>(handle_), name);

        if (!address)
        {
            const DWORD error = GetLastError();
            throw std::runtime_error("Required PhotinoX.Native export was not found: " + std::string(name) + ". Win32 error: " + std::to_string(error) + ".");
        }
#else
        dlerror();

        auto address = dlsym(handle_, name);
        const char* error = dlerror();

        if (error)
            throw std::runtime_error("Required PhotinoX.Native export was not found: " + std::string(name) + ". Loader error: " + error);
#endif

        return reinterpret_cast<T>(address);
    }

    Library::Library()
    {
#ifdef _WIN32
        handle_ = LoadLibraryW(LibraryName);

        if (!handle_)
        {
            const DWORD error = GetLastError();

            throw std::runtime_error("Failed to load PhotinoX.Native.dll. Win32 error: " + std::to_string(error) + ".");
        }
#else
        handle_ = dlopen(LibraryName, RTLD_NOW | RTLD_LOCAL);

        if (!handle_)
        {
            const char* error = dlerror();

            throw std::runtime_error(error ? error : "Failed to load PhotinoX.Native shared library.");
        }
#endif

        try
        {
            getVersion_ = LoadExport<decltype(getVersion_)>("Photino_GetNativeVersion");
            getRuntimeInfo_ = LoadExport<decltype(getRuntimeInfo_)>("Photino_GetRuntimeInfo");

            //memory
            allocateMemory_ = LoadExport<decltype(allocateMemory_)>("Photino_AllocateMemory");
            freeMemory_ = LoadExport<decltype(freeMemory_)>("Photino_FreeMemory");
            allocateString_ = LoadExport<decltype(allocateString_)>("Photino_AllocateString");
            freeString_ = LoadExport<decltype(freeString_)>("Photino_FreeString");
            freeStringArray_ = LoadExport<decltype(freeStringArray_)>("Photino_FreeStringArray");

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
            windowHide_ = LoadExport<decltype(windowHide_)>("Photino_Hide");
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

            windowGetVisible_ = LoadExport<decltype(windowGetVisible_)>("Photino_GetVisible");

            windowGetTransparentEnabled_ = LoadExport<decltype(windowGetTransparentEnabled_)>("Photino_GetTransparentEnabled");
            windowSetTransparentEnabled_ = LoadExport<decltype(windowSetTransparentEnabled_)>("Photino_SetTransparentEnabled");

            windowGetHandle_ = LoadExport<decltype(windowGetHandle_)>("Photino_GetWindowHandle");
            windowBeginDrag_ = LoadExport<decltype(windowBeginDrag_)>("Photino_BeginWindowDrag");
            windowBeginResize_ = LoadExport<decltype(windowBeginResize_)>("Photino_BeginWindowResize");
            windowGetScreenDpi_ = LoadExport<decltype(windowGetScreenDpi_)>("Photino_GetScreenDpi");
            windowGetAllMonitors_ = LoadExport<decltype(windowGetAllMonitors_)>("Photino_GetAllMonitors");
            windowGetMonitor_ = LoadExport<decltype(windowGetMonitor_)>("Photino_GetWindowMonitor");
            windowSetChromelessDragRegions_ = LoadExport<decltype(windowSetChromelessDragRegions_)>("Photino_SetChromelessDragRegions");
            windowSetChromelessResizeBorderThickness_ = LoadExport<decltype(windowSetChromelessResizeBorderThickness_)>("Photino_SetChromelessResizeBorderThickness");

            //browser
            windowNavigateToString_ = LoadExport<decltype(windowNavigateToString_)>("Photino_NavigateToString");
            windowNavigateToUrl_ = LoadExport<decltype(windowNavigateToUrl_)>("Photino_NavigateToUrl");
            windowSendWebMessage_ = LoadExport<decltype(windowSendWebMessage_)>("Photino_SendWebMessage");
            windowAddCustomSchemeName_ = LoadExport<decltype(windowAddCustomSchemeName_)>("Photino_AddCustomSchemeName");

            windowGetContextMenuEnabled_ = LoadExport<decltype(windowGetContextMenuEnabled_)>("Photino_GetContextMenuEnabled");
            windowSetContextMenuEnabled_ = LoadExport<decltype(windowSetContextMenuEnabled_)>("Photino_SetContextMenuEnabled");

            windowGetZoomEnabled_ = LoadExport<decltype(windowGetZoomEnabled_)>("Photino_GetZoomEnabled");
            windowSetZoomEnabled_ = LoadExport<decltype(windowSetZoomEnabled_)>("Photino_SetZoomEnabled");

            windowGetStatusBarEnabled_ = LoadExport<decltype(windowGetStatusBarEnabled_)>("Photino_GetStatusBarEnabled");
            windowSetStatusBarEnabled_ = LoadExport<decltype(windowSetStatusBarEnabled_)>("Photino_SetStatusBarEnabled");

            windowGetDevToolsEnabled_ = LoadExport<decltype(windowGetDevToolsEnabled_)>("Photino_GetDevToolsEnabled");
            windowSetDevToolsEnabled_ = LoadExport<decltype(windowSetDevToolsEnabled_)>("Photino_SetDevToolsEnabled");

            windowGetZoom_ = LoadExport<decltype(windowGetZoom_)>("Photino_GetZoom");
            windowSetZoom_ = LoadExport<decltype(windowSetZoom_)>("Photino_SetZoom");

            windowGetGrantBrowserPermissions_ = LoadExport<decltype(windowGetGrantBrowserPermissions_)>("Photino_GetGrantBrowserPermissions");
            windowGetMediaAutoplayEnabled_ = LoadExport<decltype(windowGetMediaAutoplayEnabled_)>("Photino_GetMediaAutoplayEnabled");
            windowGetFileSystemAccessEnabled_ = LoadExport<decltype(windowGetFileSystemAccessEnabled_)>("Photino_GetFileSystemAccessEnabled");
            windowGetWebSecurityEnabled_ = LoadExport<decltype(windowGetWebSecurityEnabled_)>("Photino_GetWebSecurityEnabled");
            windowGetJavascriptClipboardAccessEnabled_ = LoadExport<decltype(windowGetJavascriptClipboardAccessEnabled_)>("Photino_GetJavascriptClipboardAccessEnabled");
            windowGetMediaStreamEnabled_ = LoadExport<decltype(windowGetMediaStreamEnabled_)>("Photino_GetMediaStreamEnabled");
            windowGetSmoothScrollingEnabled_ = LoadExport<decltype(windowGetSmoothScrollingEnabled_)>("Photino_GetSmoothScrollingEnabled");
            windowGetIgnoreCertificateErrorsEnabled_ = LoadExport<decltype(windowGetIgnoreCertificateErrorsEnabled_)>("Photino_GetIgnoreCertificateErrorsEnabled");

            windowGetUserAgent_ = LoadExport<decltype(windowGetUserAgent_)>("Photino_GetUserAgent");
            windowClearBrowserAutoFill_ = LoadExport<decltype(windowClearBrowserAutoFill_)>("Photino_ClearBrowserAutoFill");

            //dialogs
            windowShowOpenFile_ = LoadExport<decltype(windowShowOpenFile_)>("Photino_ShowOpenFile");
            windowShowOpenFolder_ = LoadExport<decltype(windowShowOpenFolder_)>("Photino_ShowOpenFolder");
            windowShowSaveFile_ = LoadExport<decltype(windowShowSaveFile_)>("Photino_ShowSaveFile");
            windowShowMessage_ = LoadExport<decltype(windowShowMessage_)>("Photino_ShowMessage");
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

    RuntimeInfo Library::GetRuntimeInfo() const noexcept
    {
        return getRuntimeInfo_();
    }

    //memory

    void* Library::AllocateMemory(int size) const noexcept
    {
        return allocateMemory_(size);
    }

    void Library::FreeMemory(void* value) const noexcept
    {
        freeMemory_(value);
    }

    char* Library::AllocateString(int size) const noexcept
    {
        return allocateString_(size);
    }

    void Library::FreeString(char* value) const noexcept
    {
        freeString_(value);
    }

    void Library::FreeStringArray(char** values, int count) const noexcept
    {
        freeStringArray_(values, count);
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

    bool Library::WindowHide(void* instance) const noexcept
    {
        return windowHide_(instance);
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

    void Library::WindowSetTitle(void* instance, Utf8String title) const noexcept
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

    void Library::WindowSetIconFile(void* instance, Utf8String iconFile) const noexcept
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

    bool Library::WindowGetVisible(void* instance) const noexcept
    {
        bool visible = false;
        windowGetVisible_(instance, &visible);
        return visible;
    }

    void* Library::WindowGetHandle(void* instance) const noexcept
    {
        return windowGetHandle_(instance);
    }

    void Library::WindowBeginDrag(void* instance) const noexcept
    {
        windowBeginDrag_(instance);
    }

    void Library::WindowBeginResize(void* instance, WindowEdge edge) const noexcept
    {
        windowBeginResize_(instance, edge);
    }

    unsigned int Library::WindowGetScreenDpi(void* instance) const noexcept
    {
        return windowGetScreenDpi_(instance);
    }

    bool Library::WindowGetAllMonitors(void* instance, GetAllMonitorsCallback callback, void* state) const noexcept
    {
        return windowGetAllMonitors_(instance, callback, state);
    }

    bool Library::WindowGetMonitor(void* instance, Monitor& monitor) const noexcept
    {
        return windowGetMonitor_(instance, &monitor);
    }

    bool Library::WindowSetChromelessDragRegions(void* instance,
                                                 const LayoutRegion* dragRegions, int dragRegionCount,
                                                 const LayoutRegion* noDragRegions, int noDragRegionCount) const noexcept
    {
        return windowSetChromelessDragRegions_(instance, dragRegions, dragRegionCount, noDragRegions, noDragRegionCount);
    }

    bool Library::WindowSetChromelessResizeBorderThickness(void* instance, int thickness) const noexcept
    {
        return windowSetChromelessResizeBorderThickness_(instance, thickness);
    }

    // appearance

    bool Library::WindowGetTransparentEnabled(void* instance) const noexcept
    {
        bool enabled = false;
        windowGetTransparentEnabled_(instance, &enabled);
        return enabled;
    }

    void Library::WindowSetTransparentEnabled(void* instance, bool enabled) const noexcept
    {
        windowSetTransparentEnabled_(instance, enabled);
    }

    // browser

    void Library::WindowNavigateToString(void* instance, Utf8String content) const noexcept
    {
        windowNavigateToString_(instance, content);
    }

    void Library::WindowNavigateToUrl(void* instance, Utf8String url) const noexcept
    {
        windowNavigateToUrl_(instance, url);
    }

    void Library::WindowSendWebMessage(void* instance, Utf8String message) const noexcept
    {
        windowSendWebMessage_(instance, message);
    }

    bool Library::WindowAddCustomSchemeName(void* instance, Utf8String scheme) const noexcept
    {
        return windowAddCustomSchemeName_(instance, scheme);
    }

    bool Library::WindowGetContextMenuEnabled(void* instance) const noexcept
    {
        bool enabled = false;
        windowGetContextMenuEnabled_(instance, &enabled);
        return enabled;
    }

    void Library::WindowSetContextMenuEnabled(void* instance, bool enabled) const noexcept
    {
        windowSetContextMenuEnabled_(instance, enabled);
    }

    bool Library::WindowGetZoomEnabled(void* instance) const noexcept
    {
        bool enabled = false;
        windowGetZoomEnabled_(instance, &enabled);
        return enabled;
    }

    void Library::WindowSetZoomEnabled(void* instance, bool enabled) const noexcept
    {
        windowSetZoomEnabled_(instance, enabled);
    }

    bool Library::WindowGetStatusBarEnabled(void* instance) const noexcept
    {
        bool enabled = false;
        windowGetStatusBarEnabled_(instance, &enabled);
        return enabled;
    }

    void Library::WindowSetStatusBarEnabled(void* instance, bool enabled) const noexcept
    {
        windowSetStatusBarEnabled_(instance, enabled);
    }

    bool Library::WindowGetDevToolsEnabled(void* instance) const noexcept
    {
        bool enabled = false;
        windowGetDevToolsEnabled_(instance, &enabled);
        return enabled;
    }

    void Library::WindowSetDevToolsEnabled(void* instance, bool enabled) const noexcept
    {
        windowSetDevToolsEnabled_(instance, enabled);
    }

    int Library::WindowGetZoom(void* instance) const noexcept
    {
        int zoom = 100;
        windowGetZoom_(instance, &zoom);
        return zoom;
    }

    void Library::WindowSetZoom(void* instance, int zoom) const noexcept
    {
        windowSetZoom_(instance, zoom);
    }

    bool Library::WindowGetGrantBrowserPermissions(void* instance) const noexcept
    {
        bool grant = false;
        windowGetGrantBrowserPermissions_(instance, &grant);
        return grant;
    }

    bool Library::WindowGetMediaAutoplayEnabled(void* instance) const noexcept
    {
        bool enabled = false;
        windowGetMediaAutoplayEnabled_(instance, &enabled);
        return enabled;
    }

    bool Library::WindowGetFileSystemAccessEnabled(void* instance) const noexcept
    {
        bool enabled = false;
        windowGetFileSystemAccessEnabled_(instance, &enabled);
        return enabled;
    }

    bool Library::WindowGetWebSecurityEnabled(void* instance) const noexcept
    {
        bool enabled = false;
        windowGetWebSecurityEnabled_(instance, &enabled);
        return enabled;
    }

    bool Library::WindowGetJavascriptClipboardAccessEnabled(void* instance) const noexcept
    {
        bool enabled = false;
        windowGetJavascriptClipboardAccessEnabled_(instance, &enabled);
        return enabled;
    }

    bool Library::WindowGetMediaStreamEnabled(void* instance) const noexcept
    {
        bool enabled = false;
        windowGetMediaStreamEnabled_(instance, &enabled);
        return enabled;
    }

    bool Library::WindowGetSmoothScrollingEnabled(void* instance) const noexcept
    {
        bool enabled = false;
        windowGetSmoothScrollingEnabled_(instance, &enabled);
        return enabled;
    }

    bool Library::WindowGetIgnoreCertificateErrorsEnabled(void* instance) const noexcept
    {
        bool enabled = false;
        windowGetIgnoreCertificateErrorsEnabled_(instance, &enabled);
        return enabled;
    }

    std::string Library::WindowGetUserAgent(void* instance) const
    {
        char* value = windowGetUserAgent_(instance);

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

    void Library::WindowClearBrowserAutoFill(void* instance) const noexcept
    {
        windowClearBrowserAutoFill_(instance);
    }

    // dialogs

    std::vector<std::string> Library::WindowShowOpenFile(
        void* instance,
        Utf8String title,
        Utf8String defaultPath,
        bool multiSelect,
        Utf8String* filters,
        int filterCount) const
    {
        int resultCount = 0;
        char** values = windowShowOpenFile_(instance, title, defaultPath, multiSelect, filters, filterCount, &resultCount);

        try
        {
            auto result = CopyStringArray(values, resultCount);
            FreeStringArray(values, resultCount);
            return result;
        }
        catch (...)
        {
            FreeStringArray(values, resultCount);
            throw;
        }
    }

    std::vector<std::string> Library::WindowShowOpenFolder(
        void* instance,
        Utf8String title,
        Utf8String defaultPath,
        bool multiSelect) const
    {
        int resultCount = 0;
        char** values = windowShowOpenFolder_(instance, title, defaultPath, multiSelect, &resultCount);

        try
        {
            auto result = CopyStringArray(values, resultCount);
            FreeStringArray(values, resultCount);
            return result;
        }
        catch (...)
        {
            FreeStringArray(values, resultCount);
            throw;
        }
    }

    std::string Library::WindowShowSaveFile(
        void* instance,
        Utf8String title,
        Utf8String defaultPath,
        Utf8String* filters,
        int filterCount,
        Utf8String defaultFileName) const
    {
        char* value = windowShowSaveFile_(instance, title, defaultPath, filters, filterCount, defaultFileName);

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

    DialogResult Library::WindowShowMessage(
        void* instance,
        Utf8String title,
        Utf8String text,
        DialogButtons buttons,
        DialogIcon icon) const noexcept
    {
        return windowShowMessage_(instance, title, text, buttons, icon);
    }
}