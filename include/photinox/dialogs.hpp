#pragma once

#include <string>
#include <vector>

namespace photinox
{
    enum class DialogResult : int
    {
        Cancel = -1,
        Ok,
        Yes,
        No,
        Abort,
        Retry,
        Ignore
    };
    static_assert(sizeof(DialogResult) == sizeof(int));

    enum class DialogButtons : int
    {
        Ok,
        OkCancel,
        YesNo,
        YesNoCancel,
        RetryCancel,
        AbortRetryIgnore
    };
    static_assert(sizeof(DialogButtons) == sizeof(int));

    enum class DialogIcon : int
    {
        Info,
        Warning,
        Error,
        Question
    };
    static_assert(sizeof(DialogIcon) == sizeof(int));

    struct FileDialogFilter
    {
        std::string name;
        std::vector<std::string> extensions;
    };
}