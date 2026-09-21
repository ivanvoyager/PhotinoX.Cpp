#include <photinox/window_collection.hpp>

#include <photinox/application.hpp>
#include <photinox/window.hpp>

#include "event_subscription.internal.hpp"

#include <eventpp/callbacklist.h>

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <utility>

namespace photinox
{
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

        using CollectionChangedHandlerList = eventpp::CallbackList<void(const WindowCollectionChangedEventArgs&)>;

        CollectionChangedHandlerList collectionChangedHandlers;

        EventSubscriptionRegistry eventSubscriptions;

        std::unordered_map<std::uint64_t, CollectionChangedHandlerList::Handle> collectionChangedHandlerSubscriptions;

        void RaiseCollectionChanged(const WindowCollectionChangedEventArgs& args) noexcept
        {
            try
            {
                collectionChangedHandlers(args);
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

    void WindowCollection::Add(Window& window)
    {
        Window* item = &window;
        AddRange(std::span<Window* const>(&item, 1));
    }

    void WindowCollection::AddRange(std::span<Window* const> windows)
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

        impl_->RaiseCollectionChanged(args);
    }

    bool WindowCollection::Remove(Window& window)
    {
        {
            std::lock_guard lock(impl_->windowsMutex);

            const auto iterator = std::find(impl_->windows.begin(), impl_->windows.end(), &window);

            if (iterator == impl_->windows.end())
                return false;

            impl_->windows.erase(iterator);
        }

        Window* item = &window;

        WindowCollectionChangedEventArgs args
        {
            .action = NotifyCollectionChangedAction::Remove,
            .newItems = {},
            .oldItems = std::span<Window* const>(&item, 1)
        };

        impl_->RaiseCollectionChanged(args);
        return true;
    }

    void WindowCollection::RemoveRange(std::span<Window* const> windows)
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

        impl_->RaiseCollectionChanged(args);
    }

    // Event subscription methods

    // Collection Changed Handlers

    WindowCollection& WindowCollection::RegisterCollectionChangedHandler(WindowCollectionChangedHandler handler)
    {
        RegisterEventHandler(impl_->collectionChangedHandlers, std::move(handler));
        return *this;
    }

    EventToken WindowCollection::SubscribeCollectionChangedHandler(WindowCollectionChangedHandler handler)
    {
        return impl_->eventSubscriptions.Subscribe(impl_->collectionChangedHandlers, impl_->collectionChangedHandlerSubscriptions, std::move(handler));
    }

    bool WindowCollection::UnsubscribeCollectionChangedHandler(EventToken token)
    {
        return impl_->eventSubscriptions.Unsubscribe(impl_->collectionChangedHandlers, impl_->collectionChangedHandlerSubscriptions, token);
    }
}