#include <photinox/photinox.hpp>

#include <iostream>

int main()
{
    photinox::Application application;

    std::cout << application.NativeVersion() << '\n';
    std::cout << std::boolalpha << application.IsRunning() << '\n';

    return 0;
}