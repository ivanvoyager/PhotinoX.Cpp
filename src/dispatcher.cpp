#include <photinox/dispatcher.hpp>

#include "native/library.hpp"

#include "event_subscription.internal.hpp"

#include <eventpp/callbacklist.h>

#include <cassert>
#include <cstdint>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <utility>

namespace photinox
{
    namespace
    {
        struct InvokeState final
        {
            DispatcherCallback callback;
            std::exception_ptr exception = nullptr;
            const Dispatcher* dispatcher = nullptr;
        };

        template<typename TCallback>
        void ValidateCallback(const TCallback& callback)
        {
            if (!callback)
                throw std::invalid_argument("callback");
        }
    } // namespace

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

        using UnhandledExceptionHandlerList = eventpp::CallbackList<void(std::exception_ptr)>;

        UnhandledExceptionHandlerList unhandledExceptionHandlers;

        EventSubscriptionRegistry eventSubscriptions;

        std::unordered_map<std::uint64_t, UnhandledExceptionHandlerList::Handle> unhandledExceptionHandlerSubscriptions;
    };

    Dispatcher::Dispatcher(native::Library& library)
        : impl_(std::make_unique<Impl>(library))
    {
    }

    Dispatcher::~Dispatcher() = default;

    void Dispatcher::OnUnhandledException(std::exception_ptr exception) const noexcept
    {
        try
        {
            impl_->unhandledExceptionHandlers(std::move(exception));
        }
        catch (...)
        {
        }
    }

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

    void Dispatcher::InvokeCallback(void* state) noexcept
    {
        auto* invokeState = static_cast<InvokeState*>(state);

        try
        {
            invokeState->callback();
        }
        catch (...)
        {
            invokeState->exception = std::current_exception();
        }
    }

    void Dispatcher::BeginInvokeCallback(void* state) noexcept
    {
        std::unique_ptr<InvokeState> invokeState(static_cast<InvokeState*>(state));

        try
        {
            invokeState->callback();
        }
        catch (...)
        {
            if (invokeState->dispatcher)
                invokeState->dispatcher->OnUnhandledException(std::current_exception());
        }
    }

    void Dispatcher::ReleaseInvokeState(void* state) noexcept
    {
        delete static_cast<InvokeState*>(state);
    }

    void Dispatcher::Invoke(DispatcherCallback callback) const
    {
        ValidateCallback(callback);

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
        ValidateCallback(callback);

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
        ValidateCallback(callback);

        auto state = std::make_unique<InvokeState>();
        state->callback = std::move(callback);
        state->dispatcher = this;

        if (!impl_->library.ApplicationBeginInvoke(BeginInvokeCallback, ReleaseInvokeState, state.get()))
            return false;

        state.release();
        return true;
    }

    // Event subscription methods

    // Unhandled Exception Handlers

    Dispatcher& Dispatcher::RegisterUnhandledExceptionHandler(UnhandledExceptionHandler handler)
    {
        RegisterEventHandler(impl_->unhandledExceptionHandlers, std::move(handler));
        return *this;
    }

    EventToken Dispatcher::SubscribeUnhandledExceptionHandler(UnhandledExceptionHandler handler)
    {
        return impl_->eventSubscriptions.Subscribe(impl_->unhandledExceptionHandlers, impl_->unhandledExceptionHandlerSubscriptions, std::move(handler));
    }

    bool Dispatcher::UnsubscribeUnhandledExceptionHandler(EventToken token)
    {
        return impl_->eventSubscriptions.Unsubscribe(impl_->unhandledExceptionHandlers, impl_->unhandledExceptionHandlerSubscriptions, token);
    }
}