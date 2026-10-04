#include <photinox/photinox.hpp>

#include <exception>
#include <iostream>
#include <string_view>

using namespace photinox;

int main()
{
    try
    {
        Application application;
        Window window(application);

        application
            .SetNotificationsEnabled(false);

        window
            .SetTitle("Transparency Demo")
            .SetChromeless(true)
            .SetSize(720, 620)
            .Center()
            .SetTransparent(true)
            .Load("wwwroot/index.html")
            .RegisterInitialContentLoadedHandler(
                [&](const ContentLoadedEventArgs&)
                {
                    window.SendWebMessage(window.Transparent() ? "transparent" : "opaque");
                })
            .RegisterWebMessageReceivedHandler(
                [&](const WebMessageReceivedEventArgs& args)
                {
                    if (args.message == "transparent")
                    {
                        window.SetTransparent(true);
                    }
                    else if (args.message == "opaque")
                    {
                        window.SetTransparent(false);
                    }
                    else if (args.message == "toggle")
                    {
                        window.SetTransparent(!window.Transparent());
                    }
                    else if (args.message == "close")
                    {
                        window.Close();
                    }

                    if (!window.IsClosed())
                    {
                        window.SendWebMessage(window.Transparent() ? "transparent" : "opaque");
                    }
                });

        return application.Run(&window);
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return 1;
    }
}