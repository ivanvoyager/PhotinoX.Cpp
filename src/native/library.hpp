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

    private:
        void* handle_ = nullptr;
        const char* (*getVersion_)() = nullptr;
    };
}