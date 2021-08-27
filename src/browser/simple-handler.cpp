#include "simple-handler.h"
#include "../utils.h"

#include <sstream>
#include <string>

#include "include/base/cef_bind.h"
#include "include/cef_app.h"
#include "include/cef_parser.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_closure_task.h"
#include "include/wrapper/cef_helpers.h"

#include <windows.h>

namespace
{
SimpleHandler *g_instance = nullptr;

// Returns a data: URI with the specified contents.
std::string GetDataURI(const std::string &data, const std::string &mime_type)
{
    return "data:" + mime_type + ";base64," +
           CefURIEncode(CefBase64Encode(data.data(), data.size()), false).ToString();
}

}  // namespace

SimpleHandler::SimpleHandler(bool use_views): is_closing_(false)
{
    DCHECK(!g_instance);
    g_instance = this;
}

SimpleHandler::~SimpleHandler() { g_instance = nullptr; }

// static
SimpleHandler *SimpleHandler::GetInstance() { return g_instance; }

void SimpleHandler::OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString &title)
{
    CEF_REQUIRE_UI_THREAD();

    PlatformTitleChange(browser, title);
}

void SimpleHandler::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    // Add to the list of existing browsers.
    browser_list_.push_back(browser);
}

bool SimpleHandler::DoClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    // Closing the main window requires special handling. See the DoClose()
    // documentation in the CEF header for a detailed destription of this
    // process.
    if (browser_list_.size() == 1) {
        // Set a flag to indicate that the window close should be allowed.
        is_closing_ = true;
    }

    // Allow the close. For windowed browsers this will result in the OS close
    // event being sent.
    return false;
}

void SimpleHandler::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    // Remove from the list of existing browsers.
    BrowserList::iterator bit = browser_list_.begin();
    for (; bit != browser_list_.end(); ++bit) {
        if ((*bit)->IsSame(browser)) {
            browser_list_.erase(bit);
            break;
        }
    }

    if (browser_list_.empty()) {
        // All browser windows have closed. Quit the application message loop.
        CefQuitMessageLoop();
    }
}

void SimpleHandler::OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                ErrorCode errorCode, const CefString &errorText,
                                const CefString &failedUrl)
{
    CEF_REQUIRE_UI_THREAD();

    // Don't display an error for downloaded files.
    if (errorCode == ERR_ABORTED) return;

    // Display a load error message using a data: URI.
    std::stringstream ss;
    ss << "<html><body bgcolor=\"white\">"
          "<h2>Failed to load URL "
       << std::string(failedUrl) << " with error " << std::string(errorText) << " (" << errorCode
       << ").</h2></body></html>";

    frame->LoadURL(GetDataURI(ss.str(), "text/html"));
}

void SimpleHandler::CloseAllBrowsers(bool force_close)
{
    if (!CefCurrentlyOn(TID_UI)) {
        // Execute on the UI thread.
        CefPostTask(TID_UI, base::Bind(&SimpleHandler::CloseAllBrowsers, this, force_close));
        return;
    }

    if (browser_list_.empty()) return;

    BrowserList::const_iterator it = browser_list_.begin();
    for (; it != browser_list_.end(); ++it) (*it)->GetHost()->CloseBrowser(force_close);
}

void SimpleHandler::PlatformTitleChange(CefRefPtr<CefBrowser> browser, const CefString &title)
{
    CefWindowHandle hwnd = browser->GetHost()->GetWindowHandle();
    if (hwnd) SetWindowText(hwnd, std::wstring(title).c_str());
}

bool SimpleHandler::OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent &event,
                               CefEventHandle os_event)
{
    CEF_REQUIRE_UI_THREAD();
    if (event.type == KEYEVENT_RAWKEYDOWN) {
        if (event.windows_key_code == VK_F5) {
            browser->Reload();
            return true;
        }
        if (event.windows_key_code == VK_LEFT && (event.modifiers & EVENTFLAG_ALT_DOWN)) {
            browser->GoBack();
            return true;
        }
        if (event.windows_key_code == VK_RIGHT && (event.modifiers & EVENTFLAG_ALT_DOWN)) {
            browser->GoForward();
            return true;
        }
    }
    return false;
}

bool SimpleHandler::OnOpenURLFromTab(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                     const CefString &target_url,
                                     CefRequestHandler::WindowOpenDisposition target_disposition,
                                     bool user_gesture)
{
    CEF_REQUIRE_UI_THREAD();
    return false;
}

bool SimpleHandler::OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                                  const CefString &target_url, const CefString &target_frame_name,
                                  CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                                  bool user_gesture, const CefPopupFeatures &popupFeatures,
                                  CefWindowInfo &windowInfo, CefRefPtr<CefClient> &client,
                                  CefBrowserSettings &settings,
                                  CefRefPtr<CefDictionaryValue> &extra_info,
                                  bool *no_javascript_access)
{
    CEF_REQUIRE_UI_THREAD();
    browser->GetMainFrame()->LoadURL(target_url);
    return true;
}

void SimpleHandler::OnDocumentAvailableInMainFrame(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();
    if (!browser->GetHost()->HasDevTools()) {
        CefBrowserSettings browser_settings;
        CefWindowInfo window_info;
        HWND parent = browser->GetHost()->GetWindowHandle();
        window_info.SetAsPopup(parent, "DevTools");
        RECT rect;
        GetWindowRect(parent, &rect);
        window_info.height = (rect.bottom - rect.top) / 2;
        window_info.width = (rect.right - rect.left) / 2;
        CefPoint pt(0, 0);
        browser->GetHost()->ShowDevTools(window_info, nullptr, browser_settings, pt);
    }
    CefRefPtr<CefFrame> frame = browser->GetMainFrame();
    frame->ExecuteJavaScript(utils::readFile(L"resources/browser/loader.js").second,
                             frame->GetURL(), 0);
}
