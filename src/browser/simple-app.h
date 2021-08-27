#pragma once
#include "include/cef_app.h"

class SimpleApp: public CefApp, public CefBrowserProcessHandler, public CefRenderProcessHandler
{
public:
    SimpleApp();

    CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override { return this; }
    CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override { return this; }

    void OnContextInitialized() override;

private:
    IMPLEMENT_REFCOUNTING(SimpleApp);
};
