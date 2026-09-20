#include <photinox/window_collection.hpp>

#include <photinox/application.hpp>
#include <photinox/window.hpp>

#include "event_token.internal.hpp"

#include <eventpp/callbacklist.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace photinox
{
    namespace
    {
        void ValidateHandler(const WindowCollectionChangedHandler& handler)
        {
            if (!handler)
                throw std::invalid_argument("handler");
        }
    }

    class WindowCollection::Impl final
    {
    public:
        explicit Impl(Application& application)
            : application(application)
        {
        }

        Application& application;

        mutable std::mutex windowsMutex;
        std::vector<Window*> windows;

        using ChangedHandlerList = eventpp::CallbackList<void(const WindowCollectionChangedEventArgs&)>;

        ChangedHandlerList changedHandlers;

        std::mutex eventSubscriptionsMutex;
        const std::uint64_t eventOwnerId = NextEventOwnerId();
        std::uint64_t nextEventToken = 1;

        std::unordered_map<std::uint64_t, ChangedHandlerList::Handle> changedHandlerSubscriptions;

        [[nodiscard]] EventToken NextEventToken()
        {
            if (nextEventToken == 0)
                throw std::overflow_error("Window collection event token limit has been reached.");

            return EventToken(eventOwnerId, nextEventToken++);
        }

        void RaiseChanged(const WindowCollectionChangedEventArgs& args) noexcept
        {
            try
            {
                changedHandlers(args);
            }
            catch (...)
            {
                application.OnUnhandledException(std::current_exception());
            }
        }
    };

    WindowCollection::WindowCollection(Application& application)
        : impl_(std::make_unique<Impl>(application))
    {
    }

    WindowCollection::~WindowCollection() = default;

    std::size_t WindowCollection::Size() const
    {
        std::lock_guard lock(impl_->windowsMutex);
        return impl_->windows.size();
    }

    bool WindowCollection::Empty() const
    {
        std::lock_guard lock(impl_->windowsMutex);
        return impl_->windows.empty();
    }

    bool WindowCollection::Contains(const Window& window) const
    {
        std::lock_guard lock(impl_->windowsMutex);
        return std::find(impl_->windows.begin(), impl_->windows.end(), &window) != impl_->windows.end();
    }

    std::vector<Window*> WindowCollection::Snapshot() const
    {
        std::lock_guard lock(impl_->windowsMutex);
        return impl_->windows;
    }

    WindowCollection& WindowCollection::RegisterChangedHandler(WindowCollectionChangedHandler handler)
    {
        ValidateHandler(handler);
        impl_->changedHandlers.append(std::move(handler));
        return *this;
    }

    EventToken WindowCollection::SubscribeChangedHandler(WindowCollectionChangedHandler handler)
    {
        ValidateHandler(handler);

        std::lock_guard lock(impl_->eventSubscriptionsMutex);

        const EventToken token = impl_->NextEventToken();
        auto handle = impl_->changedHandlers.append(std::move(handler));

        try
        {
            impl_->changedHandlerSubscriptions.emplace(token.value_, handle);
        }
        catch (...)
        {
            const bool removed = impl_->changedHandlers.remove(handle);
            assert(removed);
            throw;
        }

        return token;
    }

    bool WindowCollection::UnsubscribeChangedHandler(EventToken token)
    {
        if (!token || token.ownerId_ != impl_->eventOwnerId)
            return false;

        std::lock_guard lock(impl_->eventSubscriptionsMutex);

        auto& subscriptions = impl_->changedHandlerSubscriptions;
        const auto iterator = subscriptions.find(token.value_);

        if (iterator == subscriptions.end())
            return false;

        const bool removed = impl_->changedHandlers.remove(iterator->second);
        assert(removed);

        subscriptions.erase(iterator);
        return removed;
    }

    void WindowCollection::Add(std::span<Window* const> windows)
    {
        if (windows.empty())
            return;

        {
            std::lock_guard lock(impl_->windowsMutex);

            for (Window* window : windows)
            {
                assert(window);
                if (!window)
                    continue;

                const auto iterator = std::find(impl_->windows.begin(), impl_->windows.end(), window);
                assert(iterator == impl_->windows.end());

                if (iterator == impl_->windows.end())
                    impl_->windows.push_back(window);
            }
        }

        WindowCollectionChangedEventArgs args
        {
            .action = NotifyCollectionChangedAction::Add,
            .newItems = windows,
            .oldItems = {}
        };

        impl_->RaiseChanged(args);
    }

    void WindowCollection::Remove(std::span<Window* const> windows)
    {
        if (windows.empty())
            return;

        {
            std::lock_guard lock(impl_->windowsMutex);

            for (Window* window : windows)
            {
                assert(window);
                if (!window)
                    continue;

                const auto iterator = std::find(impl_->windows.begin(), impl_->windows.end(), window);
                assert(iterator != impl_->windows.end());

                if (iterator != impl_->windows.end())
                    impl_->windows.erase(iterator);
            }
        }

        WindowCollectionChangedEventArgs args
        {
            .action = NotifyCollectionChangedAction::Remove,
            .newItems = {},
            .oldItems = windows
        };

        impl_->RaiseChanged(args);
    }
}