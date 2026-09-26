#pragma once

#include <photinox/event_token.hpp>

#include "event_token.internal.hpp"

#include <cassert>
#include <cstdint>
#include <mutex>
#include <stdexcept>
#include <utility>

namespace photinox
{
    namespace detail
    {
        template<typename THandler>
        void ValidateHandler(const THandler& handler)
        {
            if (!handler)
                throw std::invalid_argument("handler");
        }
    }

    template<typename THandlerList, typename THandler>
    void RegisterEventHandler(THandlerList& handlers, THandler&& handler)
    {
        detail::ValidateHandler(handler);
        handlers.append(std::forward<THandler>(handler));
    }

    class EventSubscriptionRegistry final
    {
    public:
        template<typename THandlerList, typename TSubscriptionMap, typename THandler>
        EventToken Subscribe(THandlerList& handlers, TSubscriptionMap& subscriptions, THandler&& handler)
        {
            detail::ValidateHandler(handler);

            std::lock_guard lock(mutex_);

            const EventToken token = NextToken();
            auto handle = handlers.append(std::forward<THandler>(handler));

            try
            {
                [[maybe_unused]] const auto [_, inserted] = subscriptions.emplace(token.value_, handle);
                assert(inserted);
            }
            catch (...)
            {
                [[maybe_unused]] const bool removed = handlers.remove(handle);
                assert(removed);
                throw;
            }

            return token;
        }

        template<typename THandlerList, typename TSubscriptionMap>
        bool Unsubscribe(THandlerList& handlers, TSubscriptionMap& subscriptions, EventToken token)
        {
            if (!token || token.ownerId_ != ownerId_)
                return false;

            std::lock_guard lock(mutex_);

            const auto iterator = subscriptions.find(token.value_);

            if (iterator == subscriptions.end())
                return false;

            const bool removed = handlers.remove(iterator->second);
            assert(removed);

            subscriptions.erase(iterator);
            return removed;
        }

    private:
        [[nodiscard]] EventToken NextToken() noexcept
        {
            do
            {
                ++nextToken_;
            }
            while (nextToken_ == 0);

            return EventToken(ownerId_, nextToken_);
        }

        std::mutex mutex_;
        const std::uint64_t ownerId_ = NextEventOwnerId();
        std::uint64_t nextToken_ = 0;
    };
}