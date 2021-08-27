#pragma once
#include "include/cef_client.h"

#include <list>

class SimpleHandler: public CefClient,
                     public CefDisplayHandler,
                     public CefLifeSpanHandler,
                     public CefKeyboardHandler,
                     public CefRequestHandler,
                     public CefLoadHandler
{
public:
    explicit SimpleHandler(bool use_views);
    ~SimpleHandler();

    // Provide access to the single global instance of this object.
    static SimpleHandler *GetInstance();

    // CefClient methods:
    CefRefPtr<CefDisplayHandler> GetDisplayHandler() override { return this; }
    CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override { return this; }
    CefRefPtr<CefLoadHandler> GetLoadHandler() override { return this; }
    CefRefPtr<CefKeyboardHandler> GetKeyboardHandler() override { return this; }
    CefRefPtr<CefRequestHandler> GetRequestHandler() override { return this; }

    // CefDisplayHandler methods:
    void OnTitleChange(CefRefPtr<CefBrowser> browser, const CefString &title) override;

    // CefLifeSpanHandler methods:
    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    bool DoClose(CefRefPtr<CefBrowser> browser) override;
    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;
    bool OnBeforePopup(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                       const CefString &target_url, const CefString &target_frame_name,
                       CefLifeSpanHandler::WindowOpenDisposition target_disposition,
                       bool user_gesture, const CefPopupFeatures &popupFeatures,
                       CefWindowInfo &windowInfo, CefRefPtr<CefClient> &client,
                       CefBrowserSettings &settings, CefRefPtr<CefDictionaryValue> &extra_info,
                       bool *no_javascript_access) override;
    void OnDocumentAvailableInMainFrame(CefRefPtr<CefBrowser> browser) override;

    // CefLoadHandler methods:
    void OnLoadError(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame, ErrorCode errorCode,
                     const CefString &errorText, const CefString &failedUrl) override;

    // CefKeyboardHandler methods:
    bool OnKeyEvent(CefRefPtr<CefBrowser> browser, const CefKeyEvent &event,
                    CefEventHandle os_event) override;

    // CefRequestHandler methods:
    bool OnOpenURLFromTab(CefRefPtr<CefBrowser> browser, CefRefPtr<CefFrame> frame,
                          const CefString &target_url,
                          CefRequestHandler::WindowOpenDisposition target_disposition,
                          bool user_gesture) override;

    // Request that all existing browser windows close.
    void CloseAllBrowsers(bool force_close);

    bool IsClosing() const { return is_closing_; }

private:
    // Platform-specific implementation.
    void PlatformTitleChange(CefRefPtr<CefBrowser> browser, const CefString &title);

    // List of existing browser windows. Only accessed on the CEF UI thread.
    typedef std::list<CefRefPtr<CefBrowser>> BrowserList;
    BrowserList browser_list_;

    bool is_closing_;

    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(SimpleHandler);
};
