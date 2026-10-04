#include <photinox/photinox.hpp>

#include <nlohmann/json.hpp>

#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace photinox;
using Json = nlohmann::json;

namespace
{
    std::string_view ToString(WindowState state) noexcept
    {
        switch (state)
        {
            case WindowState::Normal:
                return "Normal";

            case WindowState::Minimized:
                return "Minimized";

            case WindowState::Maximized:
                return "Maximized";

            case WindowState::FullScreen:
                return "FullScreen";

            default:
                return "Unknown";
        }
    }

    std::int64_t GetTimestamp()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }

    class DiagnosticWindow final
    {
    public:
        explicit DiagnosticWindow(
            Application& application,
            DiagnosticWindow* parent = nullptr)
            : application_(application),
            window_(application, parent ? &parent->window_ : nullptr),
            id_(parent ? parent->id_ + ".child" : "main"),
            parentId_(parent ? parent->id_ : "")
        {
            RegisterHandlers();
        }

        DiagnosticWindow(const DiagnosticWindow&) = delete;
        DiagnosticWindow& operator=(const DiagnosticWindow&) = delete;

        Window& GetWindow() noexcept
        {
            return window_;
        }

    private:
        void RegisterHandlers()
        {
            window_
                .RegisterCreatingHandler([this]
                {
                    Log("Creating");
                })
                .RegisterCreatedHandler([this]
                {
                    Log("Created");
                    SendState();
                })
                .RegisterClosingHandler([this](ClosingEventArgs& args)
                {
                    args.cancel = cancelClosing_;

                    Log("Closing",
                    {
                        { "cancel", args.cancel }
                    });
                })
                .RegisterClosedHandler([this]
                {
                    LogToConsole("Closed");

                    if (!parentId_.empty())
                    {
                        LogToConsole("Window '" + id_ + "' closed.");
                    }
                })
                .RegisterLocationChangedHandler(
                    [this](const LocationChangedEventArgs& args)
                    {
                        Log("LocationChanged",
                        {
                            { "left", args.location.x },
                            { "top", args.location.y }
                        });

                        SendState();
                    })
                .RegisterSizeChangedHandler(
                    [this](const SizeChangedEventArgs& args)
                    {
                        Log("SizeChanged",
                        {
                            { "width", args.size.width },
                            { "height", args.size.height }
                        });

                        SendState();
                    })
                .RegisterActivatedHandler([this]
                {
                    Log("Activated");
                })
                .RegisterDeactivatedHandler([this]
                {
                    Log("Deactivated");
                })
                .RegisterMaximizedHandler([this]
                {
                    Log("Maximized");
                    SendState();
                })
                .RegisterMinimizedHandler([this]
                {
                    Log("Minimized");
                    SendState();
                })
                .RegisterRestoredHandler([this]
                {
                    Log("Restored");
                    SendState();
                })
                .RegisterFullScreenEnteredHandler([this]
                {
                    Log("FullScreenEntered");
                    SendState();
                })
                .RegisterFullScreenExitedHandler([this]
                {
                    Log("FullScreenExited");
                    SendState();
                })
                .RegisterStateChangedHandler(
                    [this](const StateChangedEventArgs& args)
                    {
                        Log("StateChanged",
                        {
                            {
                                "state", std::string(ToString(args.oldState)) +
                                " -> " + std::string(ToString(args.newState))
                            }
                        });

                        SendState();
                    })
                .RegisterWebMessageReceivedHandler(
                    [this](const WebMessageReceivedEventArgs& args)
                    {
                        HandleClientMessage(args.message);
                    })
                .RegisterNavigationStartingHandler(
                    [this](NavigationStartingEventArgs& args)
                    {
                        Log("NavigationStarting",
                        {
                            { "uri", args.uri },
                            { "cancel", args.cancel }
                        });
                    })
                .RegisterContentLoadingHandler(
                    [this](const ContentLoadingEventArgs& args)
                    {
                        Log("ContentLoading",
                        {
                            { "uri", args.uri }
                        });
                    })
                .RegisterContentLoadedHandler(
                    [this](const ContentLoadedEventArgs& args)
                    {
                        Log("ContentLoaded",
                        {
                            { "uri", args.uri }
                        });
                    });
        }

        void HandleClientMessage(std::string_view message)
        {
            Json clientMessage;

            try
            {
                clientMessage = Json::parse(message);
            }
            catch (const std::exception& exception)
            {
                Log("InvalidClientMessage",
                {
                    { "error", exception.what() },
                    { "message", message }
                });

                return;
            }

            const std::string type = clientMessage.value("type", std::string{});

            if (type == "ready")
            {
                clientReady_ = true;

                SendInitialPayload();
                FlushPendingEntries();
                SendState();
                return;
            }

            if (type == "action")
            {
                ExecuteAction(
                    clientMessage.value("target", std::string{}),
                    clientMessage.value("action", std::string{}));

                return;
            }

            if (type == "setCancelClosing")
            {
                cancelClosing_ = clientMessage.value("value", false);

                return;
            }

            if (type == "setLogActions")
            {
                logActions_ = clientMessage.value("value", false);
            }
        }

        void ExecuteAction(std::string_view target, std::string_view action)
        {
            if (action.empty())
                return;

            DiagnosticWindow* context = this;

            if (target == "child")
            {
                if (action == "hide")
                {
                    context = child_ && !child_->window_.IsClosed() ? child_.get() : nullptr;
                }
                else
                {
                    context = &GetOrCreateChild();
                }
            }

            if (!context)
                return;

            if (context->logActions_)
            {
                context->LogDiagnostic("Action",
                {
                    { "target", target.empty() ? "self" : std::string(target) },
                    { "action", action }
                });
            }

            try
            {
                if (action == "show")
                {
                    context->window_.Show();
                }
                else if (action == "hide")
                {
                    context->window_.Hide();
                }
                else if (action == "activate")
                {
                    if (!context->window_.Activate())
                    {
                        throw std::runtime_error("Failed to activate the window.");
                    }
                }
                else if (action == "bringToFront")
                {
                    context->window_.BringToFront();
                }
                else if (action == "maximize")
                {
                    context->window_.Maximize();
                }
                else if (action == "minimize")
                {
                    context->window_.Minimize();
                }
                else if (action == "restore")
                {
                    context->window_.Restore();
                }
                else if (action == "toggleFullScreen")
                {
                    context->window_.SetFullScreen(context->window_.GetWindowState() != WindowState::FullScreen);
                }
                else if (action == "close")
                {
                    context->window_.Close();
                }
            }
            catch (const std::exception& exception)
            {
                context->LogDiagnostic("ActionFailed",
                {
                    { "action", action },
                    { "message", exception.what() }
                });
            }

            context->SendState();
        }

        DiagnosticWindow& GetOrCreateChild()
        {
            if (child_ && child_->window_.IsClosed())
                child_.reset();

            if (child_)
                return *child_;

            child_ = std::make_unique<DiagnosticWindow>(application_, this);

            child_->window_
                .SetTitle("PhotinoX Window Diagnostics - Child of " + id_)
                .SetSize(1000, 720)
                .Center()
                .Load("wwwroot/main.html");

            child_->window_.RegisterClosedHandler([this]
            {
                const std::string childId = child_ ? child_->id_ : std::string{};

                Log("ChildClosed",
                {
                    { "childId", childId }
                });

                SendState();
            });

            SendState();
            return *child_;
        }

        void SendInitialPayload()
        {
            Send(
            {
                { "type", "init" },
                { "id", id_ },
                { "parentId", parentId_.empty() ? Json(nullptr) : Json(parentId_) },
                { "title", window_.Title() }
            });
        }

        void SendState()
        {
            if (!clientReady_ || window_.IsClosed())
                return;

            try
            {
                Size size{};
                Point location{};
                WindowState state = WindowState::Normal;
                bool visible = false;

                if (window_.IsInitialized())
                {
                    size = window_.GetSize();
                    location = window_.Location();
                    state = window_.GetWindowState();
                    visible = window_.IsVisible();
                }

                Send(
                {
                    { "type", "state" },
                    { "id", id_ },
                    { "parentId", parentId_.empty() ? Json(nullptr) : Json(parentId_) },
                    { "isInitialized", window_.IsInitialized() },
                    { "isVisible", visible },
                    { "isClosed", window_.IsClosed() },
                    {  "hasChild", child_ && !child_->window_.IsClosed() },
                    { "fullScreen", state == WindowState::FullScreen },
                    { "state", ToString(state) },
                    {
                        "size",
                        {
                            { "width", size.width },
                            { "height", size.height }
                        }
                    },
                    {
                        "location",
                        {
                            { "x", location.x },
                            { "y", location.y }
                        }
                    }
                });
            }
            catch (const std::exception& exception)
            {
                LogToConsole("Failed to send state: " + std::string(exception.what()));
            }
        }

        void Log(std::string_view eventName, Json payload = nullptr)
        {
            Json entry =
            {
                { "type", "event" },
                { "windowId", id_ },
                { "parentId", parentId_.empty() ? Json(nullptr) : Json(parentId_) },
                { "eventName", eventName },
                { "timestamp", GetTimestamp() },
                { "payload", std::move(payload) }
            };

            LogToConsole(FormatConsoleLog(entry));

            if (!clientReady_ || window_.IsClosed())
            {
                pendingEntries_.push_back(std::move(entry));
                return;
            }

            Send(entry);
        }

        void LogDiagnostic(std::string_view name, Json payload = nullptr)
        {
            Json entry =
            {
                { "type", "diagnostic" },
                { "windowId", id_ },
                { "parentId", parentId_.empty() ? Json(nullptr) : Json(parentId_) },
                { "eventName", name },
                { "timestamp", GetTimestamp() },
                { "payload", std::move(payload) }
            };

            LogToConsole(FormatConsoleLog(entry));

            if (!clientReady_ || window_.IsClosed())
            {
                pendingEntries_.push_back(std::move(entry));
                return;
            }

            Send(entry);
        }

        void FlushPendingEntries()
        {
            for (const Json& entry : pendingEntries_)
                Send(entry);

            pendingEntries_.clear();
        }

        void Send(const Json& value)
        {
            if (!window_.IsInitialized() || window_.IsClosed())
            {
                return;
            }

            window_.SendWebMessage(value.dump());
        }

        static std::string FormatConsoleLog(
            const Json& entry)
        {
            std::string result =
                std::to_string(entry.at("timestamp").get<std::int64_t>()) +
                " | " +
                entry.at("windowId").get<std::string>() +
                " | " +
                entry.at("eventName").get<std::string>();

            const Json& payload = entry.at("payload");

            if (!payload.is_null())
                result += " | " + payload.dump();

            return result;
        }

        static void LogToConsole(std::string_view message)
        {
            std::cout << message << '\n';
        }

        Application& application_;
        Window window_;
        std::string id_;
        std::string parentId_;
        std::vector<Json> pendingEntries_;
        bool clientReady_ = false;
        bool cancelClosing_ = false;
        bool logActions_ = false;
        std::unique_ptr<DiagnosticWindow> child_;
    };
}

int main()
{
    try
    {
        Application application;
        DiagnosticWindow mainWindow(application);

        application.SetNotificationsEnabled(false);

        mainWindow.GetWindow()
            .SetTitle("PhotinoX Window Diagnostics")
            .SetSize(1200, 820)
            .Center()
            .Load("wwwroot/main.html");

        return application.Run(&mainWindow.GetWindow());
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return 1;
    }
}