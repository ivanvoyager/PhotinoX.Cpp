#pragma once

namespace photinox::native
{
    class Library final
    {
    public:
        Library();
        ~Library();

        Library(const Library&) = delete;
        Library& operator=(const Library&) = delete;

        Library(Library&&) = delete;
        Library& operator=(Library&&) = delete;

        [[nodiscard]] const char* GetVersion() const noexcept;
        [[nodiscard]] bool IsApplicationRunning() const noexcept;

    private:
        template<typename T>
        [[nodiscard]] T LoadExport(const char* name) const;
        void* handle_ = nullptr;
        const char* (*getVersion_)() = nullptr;
        bool (*isApplicationRunning_)() = nullptr;
    };
}