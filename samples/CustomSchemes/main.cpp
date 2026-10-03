#include <photinox/photinox.hpp>

#include <cstddef>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

using namespace photinox;

namespace
{
    std::string GetRequestPath(std::string_view url)
    {
        const std::size_t schemeEnd = url.find("://");

        if (schemeEnd == std::string_view::npos)
            return "/index.html";

        const std::size_t pathStart = url.find('/', schemeEnd + 3);

        if (pathStart == std::string_view::npos)
            return "/index.html";

        const std::size_t queryStart = url.find_first_of("?#", pathStart);
        const std::string_view path = url.substr(pathStart, queryStart - pathStart);

        return path.empty() || path == "/" ? "/index.html" : std::string(path);
    }

    std::vector<std::byte> ToBytes(std::string_view value)
    {
        const auto* begin = reinterpret_cast<const std::byte*>(value.data());
        return { begin, begin + value.size() };
    }

    std::vector<std::byte> ReadFile(const std::filesystem::path& path)
    {
        std::ifstream stream(path, std::ios::binary | std::ios::ate);

        if (!stream)
            throw std::runtime_error("Failed to open file: " + path.string());

        const std::streampos end = stream.tellg();

        if (end < 0)
            throw std::runtime_error("Failed to get file size: " + path.string());

        std::vector<std::byte> content(static_cast<std::size_t>(end));

        stream.seekg(0);

        if (!content.empty() &&
            !stream.read(reinterpret_cast<char*>(content.data()), static_cast<std::streamsize>(content.size())))
        {
            throw std::runtime_error("Failed to read file: " + path.string());
        }

        return content;
    }

    std::string GetContentType(const std::filesystem::path& path)
    {
        const std::string extension = path.extension().string();

        if (extension == ".html")
            return "text/html";

        if (extension == ".css")
            return "text/css";

        if (extension == ".js")
            return "text/javascript";

        if (extension == ".json")
            return "application/json";

        return "application/octet-stream";
    }
}

int main(int argc, char* argv[])
{
    try
    {
        Application application;
        Window window(application);

        application
            .SetNotificationsEnabled(false)
            .SetShutdownMode(ShutdownMode::OnMainWindowClose);

        const std::filesystem::path executableDirectory =
            argc > 0
                ? std::filesystem::absolute(argv[0]).parent_path()
                : std::filesystem::current_path();

        const std::filesystem::path webRoot = executableDirectory / "wwwroot";

        window
            .SetTitle("PhotinoX Custom Schemes")
            .SetSize(900, 700)
            .Center()
            .RegisterCustomSchemeHandler(
                "app",
                [webRoot](std::string_view scheme, std::string_view url)
                {
                    std::cout
                        << "Custom scheme request. Scheme: " << scheme
                        << ", URL: " << url
                        << '\n';

                    const std::string requestPath = GetRequestPath(url);

                    if (requestPath == "/data.json")
                    {
                        constexpr std::string_view json = R"({
    "status": "ok",
    "source": "app://localhost/data.json"
})";

                        return CustomSchemeResponse
                        {
                            .content = ToBytes(json),
                            .contentType = "application/json"
                        };
                    }

                    std::filesystem::path filePath;

                    if (requestPath == "/index.html")
                        filePath = webRoot / "index.html";
                    else if (requestPath == "/style.css")
                        filePath = webRoot / "style.css";
                    else if (requestPath == "/app.js")
                        filePath = webRoot / "app.js";
                    else
                    {
                        return CustomSchemeResponse
                        {
                            .content = ToBytes("Not found: " + std::string(url)),
                            .contentType = "text/plain"
                        };
                    }

                    if (!std::filesystem::is_regular_file(filePath))
                    {
                        return CustomSchemeResponse
                        {
                            .content = ToBytes("Not found: " + std::string(url)),
                            .contentType = "text/plain"
                        };
                    }

                    return CustomSchemeResponse
                    {
                        .content = ReadFile(filePath),
                        .contentType = GetContentType(filePath)
                    };
                })
            .RegisterWebMessageReceivedHandler(
                [](const WebMessageReceivedEventArgs& args)
                {
                    std::cout << "Message from WebView: " << args.message << '\n';
                })
            .Load("app://localhost/index.html");

        return application.Run(&window);
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return 1;
    }
}