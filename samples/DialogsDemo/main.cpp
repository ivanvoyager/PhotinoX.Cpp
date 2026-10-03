#include <photinox/photinox.hpp>

#include <nlohmann/json.hpp>

#include <exception>
#include <iostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using namespace photinox;
using Json = nlohmann::json;

namespace
{
    std::string ToString(DialogResult result)
    {
        switch (result)
        {
            case DialogResult::Cancel:
                return "Cancel";
            case DialogResult::Ok:
                return "Ok";
            case DialogResult::Yes:
                return "Yes";
            case DialogResult::No:
                return "No";
            case DialogResult::Abort:
                return "Abort";
            case DialogResult::Retry:
                return "Retry";
            case DialogResult::Ignore:
                return "Ignore";
            default:
                return "Unknown";
        }
    }
}

int main()
{
    try
    {
        Application application;
        Window window(application);

        application
            .SetNotificationsEnabled(false)
            .SetShutdownMode(ShutdownMode::OnMainWindowClose);

        const std::vector<FileDialogFilter> openFilters
        {
            {
                .name = "Images",
                .extensions = { "png", "jpg", "jpeg", "gif", "webp" }
            },
            {
                .name = "Text files",
                .extensions = { "txt", "md", "json", "xml" }
            },
            {
                .name = "All files",
                .extensions = { "*" }
            }
        };

        const std::vector<FileDialogFilter> saveFilters
        {
            {
                .name = "Text files",
                .extensions = { "txt" }
            },
            {
                .name = "JSON files",
                .extensions = { "json" }
            },
            {
                .name = "All files",
                .extensions = { "*" }
            }
        };

        window
            .SetTitle("PhotinoX Dialogs Demo")
            .SetSize(1100, 760)
            .Center()
            .Load("wwwroot/index.html")
            .RegisterWebMessageReceivedHandler(
                [&window, openFilters, saveFilters](const WebMessageReceivedEventArgs& args)
                {
                    try
                    {
                        if (args.message == "open-file")
                        {
                            const auto paths = window.ShowOpenFile("Choose a file", {}, false, openFilters);

                            window.SendWebMessage(Json
                            {
                                { "type", "dialog-result" },
                                { "title", "Open file" },
                                { "paths", paths }
                            }.dump());

                            return;
                        }

                        if (args.message == "open-files")
                        {
                            const auto paths = window.ShowOpenFile("Choose files", {}, true, openFilters);

                            window.SendWebMessage(Json
                             {
                                 { "type", "dialog-result" },
                                 { "title", "Open multiple files" },
                                 { "paths", paths }
                             }.dump());

                            return;
                        }

                        if (args.message == "open-folder")
                        {
                            const auto paths = window.ShowOpenFolder("Choose a folder", {}, false);

                            window.SendWebMessage(Json
                            {
                                { "type", "dialog-result" },
                                { "title", "Open folder" },
                                { "paths", paths }
                            }.dump());

                            return;
                        }

                        if (args.message == "open-folders")
                        {
                            const auto paths = window.ShowOpenFolder("Choose folders", {}, true);

                            window.SendWebMessage(Json
                            {
                                { "type", "dialog-result" },
                                { "title", "Open multiple folders" },
                                { "paths", paths }
                            }.dump());

                            return;
                        }

                        if (args.message == "save-file")
                        {
                            const std::string path = window.ShowSaveFile("Save a text file", {}, saveFilters, "document.txt");

                            Json paths = Json::array();

                            if (!path.empty())
                                paths.push_back(path);

                            window.SendWebMessage(Json
                            {
                                { "type", "dialog-result" },
                                { "title", "Save file" },
                                { "paths", std::move(paths) }
                            }.dump());

                            return;
                        }

                        if (args.message == "show-message")
                        {
                            const DialogResult result = window.ShowMessage(
                                "PhotinoX Dialogs Demo",
                                "This message is displayed by the native operating-system dialog.",
                                DialogButtons::YesNoCancel,
                                DialogIcon::Question);

                            window.SendWebMessage(Json
                            {
                                { "type", "message-result" },
                                { "result", ToString(result) }
                            }.dump());
                        }
                    }
                    catch (const std::exception& exception)
                    {
                        if (!window.IsClosed())
                            window.SendWebMessage(Json
                            {
                                { "type", "error" },
                                { "message", exception.what() }
                            }.dump());
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