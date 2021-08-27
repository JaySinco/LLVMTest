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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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
    LOG(INFO) << "loading js!!";
    CefRefPtr<CefFrame> frame = browser->GetMainFrame();
    frame->ExecuteJavaScript(utils::readFile(L"resources/browser/loader.js").second,
                             frame->GetURL(), 0);
}

void SimpleHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect &rect)
{
    CEF_REQUIRE_UI_THREAD();
    rect.x = 0;
    rect.y = 0;
    rect.width = 1440;
    rect.height = 900;
}

void SimpleHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type,
                            const RectList &dirtyRects, const void *buffer, int width, int height)
{
    static int i = 0;
    CEF_REQUIRE_UI_THREAD();
    LOG(INFO) << "OnPaint: width=" << width << ", height=" << height;
    stbi_write_jpg("out_{}.jpg"_format(i++).c_str(), width, height, 4, buffer, 80);
    browser->GetMainFrame()->LoadURL("www.bing.com");
}

bool SimpleHandler::OnConsoleMessage(CefRefPtr<CefBrowser> browser, cef_log_severity_t level,
                                     const CefString &message, const CefString &source, int line)
{
    CEF_REQUIRE_UI_THREAD();
    LOG(INFO) << "[Console] " << message;
    return true;
}
