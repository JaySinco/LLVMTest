#include <windows.h>
#include <iostream>
#include "../utils.h"
#include "simple-app.h"

int wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    CefEnableHighDPISupport();
    CefMainArgs main_args(hInstance);
    CefRefPtr<SimpleApp> app(new SimpleApp);
    int exit_code = CefExecuteProcess(main_args, app, nullptr);
    if (exit_code >= 0) {
        return exit_code;
    }
    CefSettings settings;
    settings.no_sandbox = true;
    settings.remote_debugging_port = 8081;
    std::wstring cachePath = utils::getExePath() + L"\\cache";
    CefString(&settings.root_cache_path).FromWString(cachePath);
    CefString(&settings.cache_path).FromWString(cachePath);
    CefInitialize(main_args, settings, app, nullptr);
    CefRunMessageLoop();
    CefShutdown();
    return 0;
}
