#pragma once

#include <filesystem>
#include <string>
#include <string_view>

namespace photinox::detail
{
    [[nodiscard]] std::string GetDefaultUserDataFolder();
    [[nodiscard]] std::filesystem::path GetExecutableDirectory();
    [[nodiscard]] std::filesystem::path PathFromUtf8(std::string_view value);
    [[nodiscard]] std::string PathToFileUrl(const std::filesystem::path& path);
}