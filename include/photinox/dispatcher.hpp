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
    class Window;

    class Dispatcher final
    {
    public:
        ~Dispatcher();

        Dispatcher(const Dispatcher&) = delete;
        Dispatcher& operator=(const Dispatcher&) = delete;

        Dispatcher(Dispatcher&&) = delete;
        Dispatcher& operator=(Dispatcher&&) = delete;

        [[nodiscard]] bool CheckAccess() const;
        void VerifyAccess() const;

        void Invoke(DispatcherCallback callback) const;
        [[nodiscard]] bool TryInvoke(DispatcherCallback callback) const;
        [[nodiscard]] bool BeginInvoke(DispatcherCallback callback) const;

    private:
        friend class Application;
        friend class Window;

        class Impl;
        std::unique_ptr<Impl> impl_;

        void VerifyAccessToCreateWindow();

        explicit Dispatcher(native::Library& library);
    };
}
