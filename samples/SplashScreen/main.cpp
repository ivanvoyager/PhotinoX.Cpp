#include <photinox/photinox.hpp>

#include <chrono>
#include <exception>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>

using namespace photinox;
using namespace std::chrono_literals;

namespace
{
    class JoiningThread final
    {
    public:
        JoiningThread() = default;

        JoiningThread(const JoiningThread&) = delete;
        JoiningThread& operator=(const JoiningThread&) = delete;

        ~JoiningThread()
        {
            if (thread_.joinable())
                thread_.join();
        }

        template<typename TCallback>
        void Start(TCallback&& callback)
        {
            if (thread_.joinable())
                throw std::logic_error("The startup thread has already been started.");

            thread_ = std::thread(std::forward<TCallback>(callback));
        }

    private:
        std::thread thread_;
    };
}

int main()
{
    try
    {
        Application application;
        Window splashWindow(application);

        std::unique_ptr<Window> mainWindow;
        JoiningThread startupThread;

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
                    startupThread.Start([&]
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
                            std::cerr << "Application startup failed: " << exception.what() << '\n';

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
                    });
                });

        application.RegisterStartupHandler([&]
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