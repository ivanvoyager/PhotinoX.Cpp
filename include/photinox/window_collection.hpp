#pragma once

#include <photinox/callbacks.hpp>
#include <photinox/event_token.hpp>

#include <cstddef>
#include <memory>
#include <span>
#include <vector>

namespace photinox
{
    class Application;
    class Window;

    class WindowCollection final
    {
    public:
        ~WindowCollection();

        WindowCollection(const WindowCollection&) = delete;
        WindowCollection& operator=(const WindowCollection&) = delete;

        WindowCollection(WindowCollection&&) = delete;
        WindowCollection& operator=(WindowCollection&&) = delete;

        [[nodiscard]] std::size_t Size() const;
        [[nodiscard]] bool Empty() const;
        [[nodiscard]] bool Contains(const Window& window) const;
        [[nodiscard]] std::vector<Window*> Snapshot() const;

        WindowCollection& RegisterChangedHandler(WindowCollectionChangedHandler handler);
        [[nodiscard]] EventToken SubscribeChangedHandler(WindowCollectionChangedHandler handler);
        bool UnsubscribeChangedHandler(EventToken token);

    private:
        friend class Application;

        class Impl;
        std::unique_ptr<Impl> impl_;

        explicit WindowCollection(Application& application);

        void Add(std::span<Window* const> windows);
        void Remove(std::span<Window* const> windows);
    };
}