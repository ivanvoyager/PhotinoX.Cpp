#pragma once

#include <photinox/callbacks.hpp>

#include <memory>

namespace photinox
{
    namespace native
    {
        class Library;
    }

    class Application;

    class Dispatcher final
    {
    public:
        ~Dispatcher();

        Dispatcher(const Dispatcher&) = delete;
        Dispatcher& operator=(const Dispatcher&) = delete;

        Dispatcher(Dispatcher&&) = delete;
        Dispatcher& operator=(Dispatcher&&) = delete;

        [[nodiscard]] bool CheckAccess() const noexcept;

        [[nodiscard]] bool Invoke(InvokeStateCallback callback, void* state) const;
        [[nodiscard]] bool BeginInvoke(InvokeStateCallback callback, void* state) const;

    private:
        friend class Application;

        explicit Dispatcher(native::Library& library);

        class Impl;
        std::unique_ptr<Impl> impl_;
    };
}
