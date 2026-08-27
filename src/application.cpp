#include <photinox/application.hpp>

#include "native/library.hpp"

namespace photinox
{
    class Application::Impl final
    {
    public:
        native::Library library;
    };

    Application::Application()
        : impl_(std::make_unique<Impl>())
    {
    }

    Application::~Application() = default;

    Application::Application(Application&&) noexcept = default;

    Application& Application::operator=(Application&&) noexcept = default;

    std::string_view Application::NativeVersion() const noexcept
    {
        const char* version = impl_->library.GetVersion();
        return version ? std::string_view(version) : std::string_view();
    }
}