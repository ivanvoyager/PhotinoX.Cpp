#include <photinox/dispatcher.hpp>

#include "native/library.hpp"

#include <exception>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>

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
        std::mutex threadMutex;
        std::thread::id threadId;
    };

    namespace
    {
        struct InvokeState final
        {
            DispatcherCallback callback;
            std::exception_ptr exception;
        };

        void InvokeCallback(void* state) noexcept
        {
            auto& invokeState = *static_cast<InvokeState*>(state);

            try
            {
                invokeState.callback();
            }
            catch (...)
            {
                invokeState.exception = std::current_exception();
            }
        }

        void BeginInvokeCallback(void* state) noexcept
        {
            std::unique_ptr<InvokeState> invokeState(static_cast<InvokeState*>(state));

            try
            {
                invokeState->callback();
            }
            catch (...)
            {
            }
        }
    }

    Dispatcher::Dispatcher(native::Library& library)
        : impl_(std::make_unique<Impl>(library))
    {
    }

    Dispatcher::~Dispatcher() = default;

    bool Dispatcher::CheckAccess() const
    {
        {
            std::scoped_lock lock(impl_->threadMutex);

            if (impl_->threadId != std::thread::id{} &&
                impl_->threadId == std::this_thread::get_id())
            {
                return true;
            }
        }

        return impl_->library.ApplicationCheckAccess();
    }

    void Dispatcher::VerifyAccess() const
    {
        if (!CheckAccess())
        {
            throw std::runtime_error("The current thread does not have access to the dispatcher.");
        }
    }

    void Dispatcher::VerifyAccessToCreateWindow()
    {
        const auto currentThreadId = std::this_thread::get_id();

        std::scoped_lock lock(impl_->threadMutex);

        if (impl_->threadId == currentThreadId)
            return;

        if (impl_->threadId == std::thread::id{})
        {
            impl_->threadId = currentThreadId;
            return;
        }

        throw std::runtime_error("Photino windows must be created on the dispatcher thread.");
    }

    void Dispatcher::Invoke(DispatcherCallback callback) const
    {
        if (!callback)
            throw std::invalid_argument("callback");

        if (CheckAccess())
        {
            callback();
            return;
        }

        InvokeState state
        {
            .callback = std::move(callback)
        };

        if (!impl_->library.ApplicationInvoke(InvokeCallback, &state))
            throw std::runtime_error("Failed to schedule the callback on the dispatcher thread.");

        if (state.exception)
            std::rethrow_exception(state.exception);
    }

    bool Dispatcher::TryInvoke(DispatcherCallback callback) const
    {
        if (!callback)
            throw std::invalid_argument("callback");

        if (CheckAccess())
        {
            callback();
            return true;
        }

        InvokeState state
        {
            .callback = std::move(callback)
        };

        if (!impl_->library.ApplicationInvoke(InvokeCallback, &state))
            return false;

        if (state.exception)
            std::rethrow_exception(state.exception);

        return true;
    }

    bool Dispatcher::BeginInvoke(DispatcherCallback callback) const
    {
        if (!callback)
            throw std::invalid_argument("callback");

        auto state = std::make_unique<InvokeState>();
        state->callback = std::move(callback);

        if (!impl_->library.ApplicationBeginInvoke(BeginInvokeCallback, state.get()))
            return false;

        state.release();
        return true;
    }
}