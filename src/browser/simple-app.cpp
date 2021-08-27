#include "simple-app.h"
#include "simple-handler.h"

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"

SimpleApp::SimpleApp() {}

void SimpleApp::OnContextInitialized()
{
    CEF_REQUIRE_UI_THREAD();
    auto command_line = CefCommandLine::GetGlobalCommandLine();
    auto url = command_line->GetSwitchValue("url");
    if (url.empty()) {
        url = "https://www.baidu.com";
    }
    CefRefPtr<SimpleHandler> handler(new SimpleHandler(false));
    CefBrowserSettings browser_settings;
    CefWindowInfo window_info;
    window_info.SetAsPopup(NULL, "browser");
    CefBrowserHost::CreateBrowser(window_info, handler, url, browser_settings, nullptr, nullptr);
}
