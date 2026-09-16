#include <photinox/dispatcher.hpp>

#include "native/library.hpp"

namespace photinox
{
    class Dispatcher::Impl final
    {
    public:
        explicit Impl(native::Library& library)
            : library(library)
        {
        }

        native::Library& library;
    };

    Dispatcher::Dispatcher(native::Library& library)
        : impl_(std::make_unique<Impl>(library))
    {
    }

    Dispatcher::~Dispatcher() = default;

    bool Dispatcher::CheckAccess() const noexcept
    {
        return impl_->library.ApplicationCheckAccess();
    }

    bool Dispatcher::Invoke(InvokeStateCallback callback, void* state) const
    {
        return impl_->library.ApplicationInvoke(callback, state);
    }

    bool Dispatcher::BeginInvoke(InvokeStateCallback callback, void* state) const
    {
        return impl_->library.ApplicationBeginInvoke(callback, state);
    }
}