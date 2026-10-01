#include "platform_paths.internal.hpp"

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <ShlObj.h>

#include <stdexcept>

namespace photinox::detail
{
    std::string GetDefaultUserDataFolder()
    {
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
    }
}

#else

namespace photinox::detail
{
    std::string GetDefaultUserDataFolder()
    {
        return {};
    }
}

#endif