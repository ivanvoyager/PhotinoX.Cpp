#include <photinox/photinox.hpp>

#include <nlohmann/json.hpp>

#include <chrono>
#include <exception>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <string_view>

using namespace photinox;
using Json = nlohmann::json;

namespace
{
    std::string GetTimestamp()
    {
        const auto now = std::chrono::system_clock::now();
        const auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        const std::time_t time = std::chrono::system_clock::to_time_t(now);
        std::tm localTime{};

#ifdef _WIN32
        localtime_s(&localTime, &time);
#else
        localtime_r(&time, &localTime);
#endif

        std::ostringstream result;
        result
            << std::put_time(&localTime, "%H:%M:%S")
            << '.'
            << std::setw(3)
            << std::setfill('0')
            << milliseconds.count();

        return result.str();
    }

    std::string_view ToString(ShutdownRequestReason reason) noexcept
    {
        switch (reason)
        {
            case ShutdownRequestReason::Unknown:
                return "Unknown";
            case ShutdownRequestReason::Application:
                return "Application";
            case ShutdownRequestReason::SessionLogoff:
                return "SessionLogoff";
            case ShutdownRequestReason::SystemShutdown:
                return "SystemShutdown";
            default:
                return "Unknown";
        }
    }

    std::string_view ToString(NotificationDismissalReason reason) noexcept
    {
        switch (reason)
        {
            case NotificationDismissalReason::Unknown:
                return "Unknown";
            case NotificationDismissalReason::UserCanceled:
                return "UserCanceled";
            case NotificationDismissalReason::ApplicationHidden:
                return "ApplicationHidden";
            case NotificationDismissalReason::TimedOut:
                return "TimedOut";
            default:
                return "Unknown";
        }
    }
}

int main(int argc, char* argv[])
{
    try
    {
        Application application;
        Window window(application);

        bool webReady = false;
        bool cancelShutdownRequested = false;
        std::queue<std::string> pendingMessages;

        const std::filesystem::path executableDirectory =
            argc > 0
            ? std::filesystem::absolute(argv[0]).parent_path()
            : std::filesystem::current_path();

        const std::filesystem::path iconPath =
            executableDirectory / "assets" / "logo.png";

        auto sendToWeb = [&](std::string message)
            {
                if (!webReady || window.IsClosed())
                {
                    pendingMessages.push(std::move(message));
                    return;
                }

                try
                {
                    window.SendWebMessage(message);
                }
                catch (const std::exception&)
                {
                    webReady = false;

                    while (!pendingMessages.empty())
                        pendingMessages.pop();
                }
            };

        auto addLog = [&](std::string message)
            {
                sendToWeb(Json
                {
                    { "type", "log" },
                    { "timestamp", GetTimestamp() },
                    { "message", std::move(message) }
                }.dump());
            };

        auto sendState = [&]
            {
                sendToWeb(Json
                {
                    { "type", "state" },
                    { "applicationName", application.Name() },
                    { "notificationRegistrationId", application.NotificationRegistrationId() },
                    { "notificationsEnabled", application.NotificationsEnabled() },
                    { "isRunning", application.IsRunning() },
                    { "cancelShutdownRequested", cancelShutdownRequested }
                }.dump());
            };

        auto flushPendingMessages = [&]
            {
                if (window.IsClosed())
                    return;

                try
                {
                    while (!pendingMessages.empty())
                    {
                        window.SendWebMessage(pendingMessages.front());
                        pendingMessages.pop();
                    }
                }
                catch (const std::exception&)
                {
                    webReady = false;

                    while (!pendingMessages.empty())
                        pendingMessages.pop();
                }
            };

        application
            .SetName("NotificationDiagnostics")
            .SetIconPath(iconPath.string())
            .SetNotificationRegistrationId("PhotinoX.Cpp.NotificationDiagnostics")
            .SetNotificationsEnabled(true)
            .SetShutdownMode(ShutdownMode::OnMainWindowClose)
            .RegisterStartupHandler([&]
            {
                addLog("Application.Startup");
            })
            .RegisterShutdownRequestedHandler(
                [&](ShutdownRequestedEventArgs& args)
                {
                    args.cancel = cancelShutdownRequested;

                    addLog(
                        "Application.ShutdownRequested: Cancel=" +
                        std::string(args.cancel ? "true" : "false") +
                        ", Reason=" +
                        std::string(ToString(args.reason)));
                })
            .RegisterExitHandler(
                [](ExitEventArgs& args)
                {
                    std::cout
                        << "Application.Exit: "
                        << args.applicationExitCode
                        << '\n';
                })
            .RegisterNotificationActionActivatedHandler(
                [&](const NotificationActionActivatedEventArgs& args)
                {
                    addLog(
                        "NotificationActionActivated: NotificationId=" +
                        std::to_string(args.notificationId) +
                        ", ActionIndex=" +
                        std::to_string(args.actionIndex));
                })
            .RegisterNotificationInputActivatedHandler(
                [&](const NotificationInputActivatedEventArgs& args)
                {
                    addLog(
                        "NotificationInputActivated: NotificationId=" +
                        std::to_string(args.notificationId) +
                        ", Response=" +
                        args.response);
                })
            .RegisterNotificationActivatedHandler(
                [&](const NotificationActivatedEventArgs& args)
                {
                    addLog(
                        "NotificationActivated: " +
                        std::to_string(args.notificationId));
                })
            .RegisterNotificationDismissedHandler(
                [&](const NotificationDismissedEventArgs& args)
                {
                    addLog(
                        "NotificationDismissed: " +
                        std::to_string(args.notificationId) +
                        ", " +
                        std::string(ToString(args.reason)));
                })
            .RegisterNotificationFailedHandler(
                [&](const NotificationFailedEventArgs& args)
                {
                    addLog(
                        "NotificationFailed: " +
                        std::to_string(args.notificationId));
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
                    std::cerr
                        << "DispatcherUnhandledException: "
                        << error.what()
                        << '\n';
                }
            });

        window
            .SetTitle("Notification Diagnostics")
            .SetSize(1180, 900)
            .Center()
            .Load("wwwroot/index.html")
            .RegisterWebMessageReceivedHandler(
                [&](const WebMessageReceivedEventArgs& args)
                {
                    try
                    {
                        const Json message = Json::parse(args.message);
                        const std::string command = message.at("command").get<std::string>();

                        if (command == "ready")
                        {
                            webReady = true;
                            flushPendingMessages();
                            addLog("Web UI ready");
                            sendState();
                            return;
                        }

                        if (command == "setCancelShutdownRequested")
                        {
                            cancelShutdownRequested = message.at("enabled").get<bool>();

                            addLog(
                                "CancelShutdownRequested changed: " +
                                std::string(cancelShutdownRequested ? "true" : "false"));

                            sendState();
                            return;
                        }

                        if (command == "show")
                        {
                            const std::string title =
                                message.value("title", std::string{});

                            const std::string body =
                                message.value("body", std::string{});

                            const std::string notificationIconPath =
                                message.value("iconPath", std::string{});

                            try
                            {
                                const int notificationId = application.ShowNotification(
                                    title,
                                    body,
                                    notificationIconPath);

                                addLog(
                                    "ShowNotification: NotificationId=" +
                                    std::to_string(notificationId) +
                                    ", Title=\"" +
                                    title +
                                    "\", Body=\"" +
                                    body +
                                    "\", IconPath=\"" +
                                    notificationIconPath +
                                    "\"");
                            }
                            catch (const std::exception& exception)
                            {
                                addLog(
                                    "ShowNotification failed: " +
                                    std::string(exception.what()));
                            }

                            return;
                        }

                        if (command == "setNotificationsEnabled")
                        {
                            const bool enabled = message.at("enabled").get<bool>();

                            try
                            {
                                application.SetNotificationsEnabled(enabled);

                                addLog(
                                    "NotificationsEnabled changed: " +
                                    std::string(enabled ? "true" : "false"));
                            }
                            catch (const std::exception& exception)
                            {
                                addLog(
                                    "NotificationsEnabled change failed: " +
                                    std::string(exception.what()));
                            }

                            sendState();
                            return;
                        }

                        if (command == "shutdown")
                            application.Shutdown();
                    }
                    catch (const std::exception& exception)
                    {
                        addLog(
                            "Web message handling failed: " +
                            std::string(exception.what()));
                    }
                });

        return application.Run(&window);
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return 1;
    }
}