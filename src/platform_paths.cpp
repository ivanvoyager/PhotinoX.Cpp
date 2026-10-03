#include "platform_paths.internal.hpp"

#include <cstdint>
#include <stdexcept>
#include <utility>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ShlObj.h>

#elif defined(__linux__)

#include <array>
#include <unistd.h>

#elif defined(__APPLE__)

#include <array>
#include <mach-o/dyld.h>

#endif

namespace photinox::detail
{
    std::string GetDefaultUserDataFolder()
    {
#ifdef _WIN32
        PWSTR localAppDataPath = nullptr;
        const HRESULT result = SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, nullptr, &localAppDataPath);

        if (FAILED(result) || !localAppDataPath)
            throw std::runtime_error("Failed to get the local application data folder.");

        const int length = WideCharToMultiByte(CP_UTF8, 0, localAppDataPath, -1, nullptr, 0, nullptr, nullptr);

        if (length <= 0)
        {
            CoTaskMemFree(localAppDataPath);
            throw std::runtime_error("Failed to convert the local application data folder to UTF-8.");
        }

        std::string userDataFolder(static_cast<std::size_t>(length), '\0');

        const int convertedLength = WideCharToMultiByte(CP_UTF8, 0, localAppDataPath, -1, userDataFolder.data(), length, nullptr, nullptr);

        CoTaskMemFree(localAppDataPath);

        if (convertedLength <= 0)
            throw std::runtime_error("Failed to convert the local application data folder to UTF-8.");

        userDataFolder.pop_back();
        userDataFolder += "\\Photino";

        return userDataFolder;
#else
        return {};
#endif
    }

    std::filesystem::path GetExecutableDirectory()
    {
#ifdef _WIN32
        std::wstring path(260, L'\0');

        for (;;)
        {
            const DWORD length = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));

            if (length == 0)
                throw std::runtime_error("Failed to get the executable path.");

            if (length < path.size() - 1)
            {
                path.resize(length);
                return std::filesystem::path(path).parent_path();
            }

            path.resize(path.size() * 2);
        }
#elif defined(__linux__)
        std::array<char, 4096> path{};
        const ssize_t length = readlink("/proc/self/exe", path.data(), path.size());

        if (length <= 0 || static_cast<std::size_t>(length) == path.size())
            throw std::runtime_error("Failed to get the executable path.");

        return std::filesystem::path(std::string(path.data(), static_cast<std::size_t>(length))).parent_path();
#elif defined(__APPLE__)
        std::uint32_t size = 0;
        _NSGetExecutablePath(nullptr, &size);

        std::string path(size, '\0');

        if (_NSGetExecutablePath(path.data(), &size) != 0)
            throw std::runtime_error("Failed to get the executable path.");

        path.resize(std::char_traits<char>::length(path.c_str()));
        return std::filesystem::weakly_canonical(path).parent_path();
#else
#error Unsupported platform
#endif
    }

    std::filesystem::path PathFromUtf8(std::string_view value)
    {
#ifdef _WIN32
        if (value.empty())
            return {};

        const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), nullptr, 0);

        if (length <= 0)
            throw std::invalid_argument("The path is not valid UTF-8.");

        std::wstring wideValue(static_cast<std::size_t>(length), L'\0');

        if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), wideValue.data(), length) <= 0)
            throw std::invalid_argument("The path is not valid UTF-8.");

        return std::filesystem::path(std::move(wideValue));
#else
        return std::filesystem::path(value);
#endif
    }

    std::string PathToFileUrl(const std::filesystem::path& path)
    {
        const std::u8string utf8Path = std::filesystem::absolute(path).generic_u8string();
        const std::string_view value(reinterpret_cast<const char*>(utf8Path.data()), utf8Path.size());

        constexpr char hexDigits[] = "0123456789ABCDEF";

#ifdef _WIN32
        std::string result = "file:///";
#else
        std::string result = "file://";
#endif

        for (const unsigned char character : value)
        {
            const bool unreserved =
                (character >= 'A' && character <= 'Z') ||
                (character >= 'a' && character <= 'z') ||
                (character >= '0' && character <= '9') ||
                character == '-' ||
                character == '.' ||
                character == '_' ||
                character == '~' ||
                character == '/' ||
                character == ':';

            if (unreserved)
            {
                result += static_cast<char>(character);
                continue;
            }

            result += '%';
            result += hexDigits[character >> 4];
            result += hexDigits[character & 0x0F];
        }

        return result;
    }
}