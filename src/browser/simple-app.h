#pragma once
#include "include/cef_app.h"

// Implement application-level callbacks for the browser process.
class SimpleApp: public CefApp, public CefBrowserProcessHandler, public CefRenderProcessHandler
{
public:
    SimpleApp();

    // CefApp methods:
    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE { return this; }
    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE { return this; }

    // CefBrowserProcessHandler methods:
    void OnContextInitialized() OVERRIDE;

private:
    // Include the default reference counting implementation.
    IMPLEMENT_REFCOUNTING(SimpleApp);
};
