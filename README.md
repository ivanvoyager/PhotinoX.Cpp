[![PhotinoX Logo](https://raw.githubusercontent.com/ivanvoyager/PhotinoX/refs/heads/master/assets/photinox-logo.png)](https://github.com/ivanvoyager/PhotinoX)

# PhotinoX.Cpp

[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/ivanvoyager/PhotinoX.Cpp)
[![Build](https://github.com/ivanvoyager/PhotinoX.Cpp/actions/workflows/build.yml/badge.svg)](https://github.com/ivanvoyager/PhotinoX.Cpp/actions/workflows/build.yml)
[![License](https://img.shields.io/github/license/ivanvoyager/PhotinoX.Cpp?label=license)](https://github.com/ivanvoyager/PhotinoX.Cpp/blob/main/LICENSE)

**PhotinoX.Cpp** is a modern C++ application framework for building lightweight, native, multi-window desktop applications with Web UI technologies and operating-system WebView controls.

It is built on [PhotinoX.Native](https://github.com/ivanvoyager/PhotinoX.Native), which provides the native application runtime, operating-system windows, platform WebView integration, message loop, dispatcher, notifications, and cross-platform C ABI.

PhotinoX.Cpp follows the application model established by [PhotinoX](https://github.com/ivanvoyager/PhotinoX) while providing an idiomatic modern C++ API based on RAII, explicit ownership, strong typing, deterministic lifetime, native exception semantics, and CMake.

- **Windows:** Microsoft Edge WebView2
- **macOS:** WKWebView
- **Linux:** WebKitGTK 4.1

## What is PhotinoX.Cpp?

PhotinoX.Cpp provides the high-level application framework layer between modern C++ application code and the cross-platform runtime, windowing, dispatch, and WebView services of PhotinoX.Native.

```text
Modern C++ application
    ↓
PhotinoX.Cpp
    ├── Application
    ├── Dispatcher
    ├── Window
    ├── WindowCollection
    ├── Typed event arguments
    ├── Removable event subscriptions
    └── Native notifications
    ↓
PhotinoX.Native
    ├── Application lifetime
    ├── Native message loop
    ├── Cross-thread dispatch
    ├── Native window management
    ├── Notification integration
    └── Stable cross-platform ABI
    ↓
Operating-system WebView
    ├── Windows: WebView2
    ├── macOS: WKWebView
    └── Linux: WebKitGTK 4.1
```

The framework allows a C++ desktop application to use HTML, CSS, and JavaScript for its user interface without bundling Electron, Node.js, or an application-specific Chromium runtime.

The native browser engine is provided by the operating system. This keeps deployment size and runtime overhead significantly lower than frameworks that ship a complete browser runtime with each application.

## PhotinoX ecosystem

PhotinoX.Native provides a language-independent native foundation for idiomatic application frameworks targeting different development environments.

```text
PhotinoX.Native
├── PhotinoX
│   └── PhotinoX.App
│       └── PhotinoX.Blazor
└── PhotinoX.Cpp
```

The responsibilities of the primary layers are:

- [**PhotinoX.Native**](https://github.com/ivanvoyager/PhotinoX.Native) provides the native cross-platform application runtime, C ABI, platform windows, WebView integration, dispatcher, notifications, and operating-system-specific implementation.
- [**PhotinoX**](https://github.com/ivanvoyager/PhotinoX) provides the primary .NET application framework and serves as the reference implementation for shared PhotinoX application semantics.
- [**PhotinoX.Cpp**](https://github.com/ivanvoyager/PhotinoX.Cpp) provides an idiomatic modern C++ application framework over the same native runtime.
- [**PhotinoX.App**](https://github.com/ivanvoyager/PhotinoX.App) adds dependency injection, configuration, logging, environment services, and application composition for .NET.
- [**PhotinoX.Blazor**](https://github.com/ivanvoyager/PhotinoX.Blazor) builds on PhotinoX.App and adds Blazor hosting and integration for .NET applications.

PhotinoX and PhotinoX.Cpp share the same fundamental application semantics, including application lifetime, dispatcher behavior, window tracking, shutdown behavior, notifications, callback ownership, and exception boundaries. Each framework expresses those concepts using the conventions and capabilities of its language.

## Why PhotinoX.Cpp?

Most C++ WebView libraries expose a single WebView object or a low-level platform wrapper around a platform browser control, leaving application lifetime, dispatch, and window management to the developer.

PhotinoX.Cpp provides a higher-level desktop application framework:

- explicit application lifetime;
- application-owned native message loop;
- centralized dispatcher;
- multi-window management;
- observable application window collection;
- configurable shutdown behavior;
- typed application and window events;
- removable event subscriptions;
- native notifications with user-defined state;
- synchronous and asynchronous dispatcher operations;
- native callback ownership and cancellation;
- exception-safe ABI boundaries;
- RAII-oriented object lifetime;
- cross-platform native packaging;
- CMake integration.

The goal is to provide the convenience and consistency normally associated with high-level desktop frameworks while preserving the performance, control, deterministic lifetime, and native integration expected from modern C++.

## Features

### Application lifetime

`Application` is the explicit owner of the native desktop lifetime.

It provides:

- `Run(...)`
- `Shutdown(...)`
- `IsRunning()`
- `IsShuttingDown()`
- configurable `ShutdownMode`
- startup, shutdown-requested, and exit events
- main-window tracking
- observable application window collection
- native notification integration

```cpp
#include <photinox/photinox.hpp>

using namespace photinox;

int main()
{
    Application application;
    Window window(application);

    window
        .SetTitle("PhotinoX.Cpp")
        .LoadString(R"(
            <!DOCTYPE html>
            <html>
                <head>
                    <meta charset="utf-8">
                    <title>PhotinoX.Cpp</title>
                </head>
                <body>
                    <h1>PhotinoX.Cpp</h1>
                    <p>Native desktop applications with modern C++ and Web UI.</p>
                </body>
            </html>
        )");

    application.SetShutdownMode(ShutdownMode::OnMainWindowClose);

    return application.Run(&window);
}
```

Only one `Application` instance may exist at a time because PhotinoX.Native provides a process-wide application runtime that owns the native message loop, dispatcher, and platform application state.

### Dispatcher

`Dispatcher` centralizes access to the native application thread.

Available operations include:

- `CheckAccess()`
- `VerifyAccess()`
- `Invoke(...)`
- result-returning `Invoke(...)`
- `TryInvoke(...)`
- result-returning `TryInvoke(...)`
- `BeginInvoke(...)`
- dispatcher-level unhandled exception events

```cpp
const int result = application.GetDispatcher().Invoke([]
{
    return 42;
});
```

Asynchronous native callbacks use explicit execution and release callbacks. If a pending operation is canceled during application shutdown, its state is released without executing the user callback.

Exceptions from asynchronous dispatcher operations are forwarded to the dispatcher unhandled-exception handlers without crossing the native ABI boundary.

### Windows

Window represents a native operating-system window with an embedded platform WebView.

The current API includes:

- explicit `Show()` and `Close()`;
- initialization and closed state;
- parent-child window relationships;
- title and startup content configuration;
- creating, created, closing, and closed events;
- typed close cancellation;
- application ownership;
- dispatcher-aware operations.

```cpp
Window window(application);

window
    .SetTitle("Settings")
    .LoadString("<html><body>Settings</body></html>")
    .RegisterClosingHandler([](ClosingEventArgs& args)
    {
        args.cancel = false;
    });

window.Show();
```

Window objects use deterministic C++ lifetime rules. Destroying a window while its native instance is still active is treated as a lifetime contract violation.

### Observable window collection

`Application::Windows()` returns a `WindowCollection`, not a non-owning view into an internal container.

The collection provides:

- `Size()`
- `Empty()`
- `Contains(...)`
- stable snapshot enumeration
- collection-changed notifications
- removable collection event subscriptions
- thread-safe snapshot access

```cpp
application.Windows().RegisterCollectionChangedHandler(
    [](const WindowCollectionChangedEventArgs& args)
    {
        if (args.action == NotifyCollectionChangedAction::Add)
        {
            for (Window* window : args.newItems)
            {
                // A window was added to the application.
            }
        }
    });
```

A stable copy can be requested explicitly:

```cpp
const auto windows = application.Windows().Snapshot();

for (Window* window : windows)
{
    if (window)
    {
        // Use the current window snapshot.
    }
}
```

Collection mutations occur on the application dispatcher thread. Read operations use a synchronized snapshot model.

### Event subscriptions

PhotinoX.Cpp supports two event registration models.

Permanent fluent registration:

```cpp
application.RegisterStartupHandler([]
{
    // Called before the native application message loop starts.
});
```

Removable token-based subscriptions:

```cpp
const EventToken token = application.SubscribeStartupHandler([]
{
    // Removable startup handler.
});

application.UnsubscribeStartupHandler(token);
```

`EventToken` is a lightweight implementation-independent value type. Native event callback handles remain private and are not exposed through the public API.

The same subscription infrastructure is shared by:

- application lifecycle events;
- notification events;
- dispatcher unhandled-exception events;
- window collection events;
- window events.

### Native notifications

Notifications are owned by `Application`, not by individual windows.

```cpp
const int notificationId = application.ShowNotification(
    "PhotinoX.Cpp",
    "The application has started.",
    {},
    std::string("startup-notification"));
```

`ShowNotification(...)` returns:

- a positive notification correlation identifier when accepted;
- 0 when not shown because of policy or application state;
- -1 for an invalid or untracked request;
- -2 when notification backend initialization fails;
- -3 when native notification display fails synchronously.

Notification events include:

- activation;
- action activation;
- input activation;
- dismissal;
- asynchronous failure.

Optional user state is stored using `std::any` and returned through the terminal notification event.

```cpp
application.RegisterNotificationActivatedHandler(
    [](const NotificationActivatedEventArgs& args)
    {
        if (const auto* value = std::any_cast<std::string>(&args.state))
        {
            // Use the associated notification state.
        }
    });
```

Notification state is released when a terminal callback is received, when the request is rejected, or when the application finishes running.

### Exception model

PhotinoX.Cpp prevents C++ exceptions from crossing the native C ABI boundary.

Synchronous lifecycle callback exceptions are handled as application lifecycle failures:

```text
Startup / ShutdownRequested / Exit
    ↓
native trampoline catches the exception
    ↓
the first exception is retained
    ↓
the application completes or shuts down
    ↓
Application::Run() rethrows the original exception
```

Asynchronous callback exceptions are forwarded through the dispatcher:

```text
BeginInvoke / notification handler / window collection handler
    ↓
native or framework callback catches the exception
    ↓
Dispatcher::UnhandledException
    ↓
the application continues running
```

This preserves structured exception propagation for synchronous application lifetime operations while keeping asynchronous event failures isolated from the native runtime.

## Modern C++ design

PhotinoX.Cpp follows modern C++ conventions instead of reproducing the .NET surface mechanically.

The API uses:

- RAII and deterministic destruction;
- deleted copy and move operations for native lifetime objects;
- `std::unique_ptr` for implementation ownership;
- `std::string_view` for synchronous non-owning string inputs and returned configuration views;
- `std::span` for temporary contiguous callback payloads;
- `std::vector` for stable collection snapshots;
- `std::any` for optional user-defined notification state;
- `std::exception_ptr` for deferred and asynchronous exception propagation;
- `std::atomic` for cross-thread lifecycle state;
- typed `enum class` values;
- explicit `[[nodiscard]]` contracts;
- `noexcept` where operations are guaranteed not to throw;
- internal PImpl-based implementation isolation;
- CMake package integration.

Shared behavior is aligned with PhotinoX, but language-specific API design is intentionally preserved.

Examples:

- .NET events map to explicit C++ register, subscribe, and unsubscribe methods.
- .NET `object` notification state maps to `std::any`.
- managed observable window collections map to synchronized C++ snapshots and typed collection events.
- asynchronous exception events carry `std::exception_ptr`.
- native handles and eventpp implementation details remain private.

## Quick start

### CMake

```cmake
include(FetchContent)

FetchContent_Declare(
    PhotinoXCpp
    GIT_REPOSITORY https://github.com/ivanvoyager/PhotinoX.Cpp.git
    GIT_TAG main
)

FetchContent_MakeAvailable(PhotinoXCpp)

add_executable(MyPhotinoApp main.cpp)
target_link_libraries(MyPhotinoApp PRIVATE PhotinoX::Cpp)
```

### Application

```cpp
#include <photinox/photinox.hpp>

#include <iostream>

using namespace photinox;

int main()
{
    Application application;
    Window window(application);

    application
        .SetName("My PhotinoX Application")
        .SetShutdownMode(ShutdownMode::OnMainWindowClose)
        .RegisterStartupHandler([]
        {
            std::cout << "Application started.\n";
        })
        .RegisterExitHandler([](ExitEventArgs& args)
        {
            std::cout << "Application exiting with code "
                      << args.applicationExitCode
                      << ".\n";
        });

    application.GetDispatcher().RegisterUnhandledExceptionHandler(
        [](std::exception_ptr exception)
        {
            try
            {
                if (exception)
                    std::rethrow_exception(exception);
            }
            catch (const std::exception& error)
            {
                std::cerr << "Unhandled dispatcher exception: "
                          << error.what()
                          << '\n';
            }
        });

    window
        .SetTitle("My PhotinoX Application")
        .LoadString(R"(
            <!DOCTYPE html>
            <html>
                <head>
                    <meta charset="utf-8">
                    <title>My PhotinoX Application</title>
                </head>
                <body>
                    <h1>Hello from PhotinoX.Cpp</h1>
                </body>
            </html>
        )");

    return application.Run(&window);
}
```

## Platform architecture

PhotinoX.Cpp uses the same PhotinoX.Native ABI on all supported desktop platforms.

| Platform | Native window system | WebView                 |
| -------- | -------------------- | ----------------------- |
| Windows  | Win32                | Microsoft Edge WebView2 |
| macOS    | Cocoa                | WKWebView               |
| Linux    | GTK 3                | WebKitGTK 4.1           |

PhotinoX.Native owns the native application runtime and platform-specific implementation. PhotinoX.Cpp provides the C++ application framework and does not expose platform implementation details through its primary API.

## Supported platforms

| Operating system | Architecture | Native runtime        |
| ---------------- | ------------ | --------------------- |
| Windows          | x64          | PhotinoX.Native.dll   |
| Windows          | ARM64        | PhotinoX.Native.dll   |
| Linux            | x64          | PhotinoX.Native.so    |
| Linux            | arm64        | PhotinoX.Native.so    |
| macOS            | x64          | PhotinoX.Native.dylib |
| macOS            | arm64        | PhotinoX.Native.dylib |

The native runtime is resolved from the PhotinoX.Native package and copied next to the application executable by the CMake integration.

## Requirements

### Common

- CMake 3.25 or later
- C++20-compatible compiler
- PhotinoX.Native package for the target platform

### Windows

- Visual Studio 2022 or later with C++ desktop tools
- Microsoft Edge WebView2 Runtime

### Linux

- GCC or Clang with C++20 support
- GTK 3 runtime
- WebKitGTK 4.1 runtime
- libnotify runtime
- a desktop notification service for native notifications

For Ubuntu-based environments:

```bash
sudo apt-get update
sudo apt-get install \
    libgtk-3-0 \
    libwebkit2gtk-4.1-0 \
    libnotify4
```

### macOS

- Xcode with a C++20-compatible Apple Clang toolchain
- WKWebView provided by the system WebKit framework

## Building from source

### Configure

```bash
cmake -S . -B build/release \
    -DCMAKE_BUILD_TYPE=Release \
    -DPHOTINOX_BUILD_SAMPLES=ON
```

### Build

```bash
cmake --build build/release --config Release
```

## Samples

See:
- [Samples](https://github.com/ivanvoyager/PhotinoX.Cpp/tree/main/samples)

## Relationship to Photino

Photino originally demonstrated that lightweight desktop applications could host Web UI through operating-system WebViews without bundling Electron.

The original Photino documentation also described Photino.Native as a foundation that could support language wrappers for C++, Rust, Go, Java, Objective-C, and other environments.

PhotinoX continues that architectural direction as an independently maintained ecosystem:

- PhotinoX.Native evolves the native application runtime and ABI.
- PhotinoX provides the managed reference implementation.
- PhotinoX.Cpp provides a native modern C++ application framework.
- Future language frameworks can target the same application-oriented native ABI.

PhotinoX.Cpp builds on the evolving PhotinoX.Native runtime and follows the shared application semantics established by PhotinoX, while expressing them through an idiomatic modern C++ API.

## Project status

PhotinoX.Cpp is under active development. The public API may change while the initial framework surface and packaging model are being completed.

The current focus is:

- expanding typed native window and WebView events;
- maintaining semantic alignment with PhotinoX;
- validating lifecycle and dispatch behavior across Windows, macOS, and Linux;
- stabilizing CMake packaging and native runtime resolution;
- preparing the framework as a foundation for production desktop applications.

## Contributing

Issues and PRs are welcome. Keep PRs focused, minimal, and consistent with the rest of PhotinoX.Cpp.

## License

PhotinoX.Cpp is licensed under Apache-2.0.

PhotinoX.Cpp is an independent project built on the independently maintained PhotinoX.Native fork. It is not affiliated with the original Photino organization.