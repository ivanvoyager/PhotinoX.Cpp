#include <photinox/photinox.hpp>

#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <thread>

using namespace photinox;
using namespace std::chrono_literals;

int main()
{
    try
    {
        Application application;
        Window splashWindow(application);

        std::unique_ptr<Window> mainWindow;
        std::jthread startupThread;

        application
            .SetShutdownMode(ShutdownMode::OnMainWindowClose)
            .SetNotificationsEnabled(false);

        splashWindow
            .SetTitle("PhotinoX")
            .SetChromeless(true)
            .SetResizable(false)
            .SetSize(480, 300)
            .Center()
            .Load("wwwroot/splash.html")
            .RegisterInitialContentLoadedHandler(
                [&](const ContentLoadedEventArgs&)
                {
                    startupThread = std::jthread(
                        [&](std::stop_token)
                        {
                            try
                            {
                                std::this_thread::sleep_for(100ms);

                                if (!application.GetDispatcher().BeginInvoke([&]
                                    {
                                        splashWindow.Show();
                                    }))
                                {
                                    throw std::runtime_error("Failed to schedule the splash window display.");
                                }

                                std::this_thread::sleep_for(1500ms); // Simulate application startup work.

                                if (!application.GetDispatcher().BeginInvoke([&]
                                    {
                                        mainWindow = std::make_unique<Window>(application);

                                        mainWindow
                                            ->SetTitle("PhotinoX Splash Screen Sample")
                                            .SetSize(1000, 800)
                                            .Center()
                                            .Load("wwwroot/index.html");

                                        application.SetMainWindow(mainWindow.get());

                                        mainWindow->Show();
                                        splashWindow.Close();
                                    }))
                                {
                                    throw std::runtime_error("Failed to schedule the main window creation.");
                                }
                            }
                            catch (const std::exception& exception)
                            {
                                std::cerr
                                    << "Application startup failed: "
                                    << exception.what()
                                    << '\n';

                                try
                                {
                                    if (!application.GetDispatcher().BeginInvoke([&]
                                        {
                                            if (!splashWindow.IsClosed())
                                                splashWindow.Close();

                                            application.Shutdown(1, true);
                                        }))
                                    {
                                        std::cerr << "Failed to schedule application shutdown.\n";
                                    }
                                }
                                catch (...)
                                {
                                }
                            }
                        });
                });

        application.RegisterStartupHandler(
            [&]
            {
                splashWindow.Initialize();
            });

        return application.Run();
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << '\n';
        return 1;
    }
}