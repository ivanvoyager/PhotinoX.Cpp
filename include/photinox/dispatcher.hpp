#pragma once

#include <photinox/callbacks.hpp>
#include <photinox/event_token.hpp>

#include <concepts>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

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
        template<typename TCallback>
        requires std::invocable<TCallback&> &&
                 (!std::is_void_v<std::invoke_result_t<TCallback&>>) &&
                 (!std::is_reference_v<std::invoke_result_t<TCallback&>>)
        [[nodiscard]] auto Invoke(TCallback callback) const -> std::invoke_result_t<TCallback&>
        {
            using TResult = std::invoke_result_t<TCallback&>;

            if (CheckAccess())
                return std::invoke(callback);

            std::optional<TResult> result;

            Invoke(DispatcherCallback([&callback, &result]
                                      {
                                          result.emplace(std::invoke(callback));
                                      }));

            if (!result)
                throw std::runtime_error("The dispatcher callback did not produce a result.");

            return std::move(*result);
        }
        [[nodiscard]] bool TryInvoke(DispatcherCallback callback) const;
        template<typename TCallback, typename TResult>
        requires std::invocable<TCallback&>&&
                 std::same_as<std::invoke_result_t<TCallback&>, TResult> &&
                 (!std::is_void_v<TResult>) &&
                 (!std::is_reference_v<TResult>)
        [[nodiscard]] bool TryInvoke(TCallback callback, TResult& result) const
        {
            if (CheckAccess())
            {
                result = std::invoke(callback);
                return true;
            }

            std::optional<TResult> invokeResult;

            const bool success = TryInvoke(DispatcherCallback([&callback, &invokeResult]
                                                              {
                                                                  invokeResult.emplace(std::invoke(callback));
                                                              }));

            if (!success)
                return false;

            if (!invokeResult)
                throw std::runtime_error("The dispatcher callback did not produce a result.");

            result = std::move(*invokeResult);
            return true;
        }
        [[nodiscard]] bool BeginInvoke(DispatcherCallback callback) const;

        Dispatcher& RegisterUnhandledExceptionHandler(UnhandledExceptionHandler handler);
        [[nodiscard]] EventToken SubscribeUnhandledExceptionHandler(UnhandledExceptionHandler handler);
        bool UnsubscribeUnhandledExceptionHandler(EventToken token);

    private:
        friend class Application;
        friend class Window;

        class Impl;
        std::unique_ptr<Impl> impl_;

        explicit Dispatcher(native::Library& library);

        void OnUnhandledException(std::exception_ptr exception) const noexcept;

        static void InvokeCallback(void* state) noexcept;
        static void BeginInvokeCallback(void* state) noexcept;
        static void ReleaseInvokeState(void* state) noexcept;

        void VerifyAccessToCreateWindow();
    };
}