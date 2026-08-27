#include <photinox/photinox.hpp>

#include <iostream>

int main()
{
    photinox::Application application;

    std::cout << application.NativeVersion() << '\n';

    return 0;
}