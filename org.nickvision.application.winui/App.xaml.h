#ifndef APP_H
#define APP_H

#include "includes.h"
#include <memory>
#include "App.xaml.g.h"
#include "controllers/mainwindowcontroller.h"

namespace winrt::Nickvision::Application::WinUI::implementation 
{
    /**
     * @brief The main WinUI application point.
     */
	class App : public AppT<App>
	{
	public:
        /**
         * @brief Constructs an app. 
         */
		App();
        /**
         * @brief Handles when the application is launched.
         * @param args Microsoft::UI::Xaml::LaunchActivatedEventArgs
         */
		void OnLaunched(const Microsoft::UI::Xaml::LaunchActivatedEventArgs& args) noexcept;

	private:
		std::shared_ptr<::Nickvision::Application::Shared::Controllers::MainWindowController> m_controller;
	};
}

#endif //APP_H