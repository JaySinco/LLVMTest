#pragma once
#include "include/cef_app.h"

// Implement application-level callbacks for the browser process.
class SimpleApp: public CefApp, public CefBrowserProcessHandler
{
public:
    SimpleApp();

    // CefApp methods:
    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE { return this; }

    // CefBrowserProcessHandler methods:
    void OnContextInitialized() OVERRIDE;
    CefRefPtr<CefClient> GetDefaultClient() OVERRIDE;

private:
    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(SimpleApp);
};
