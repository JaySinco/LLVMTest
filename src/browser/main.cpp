#include <windows.h>
#include <iostream>
#include "../utils.h"
#include "simple-app.h"

int main()
{
    CefEnableHighDPISupport();
    CefMainArgs main_args;
    CefRefPtr<SimpleApp> app(new SimpleApp);
    int exit_code = CefExecuteProcess(main_args, app, nullptr);
    if (exit_code >= 0) {
        return exit_code;
    }
    CefSettings settings;
    settings.no_sandbox = true;
    settings.windowless_rendering_enabled = true;
    settings.log_severity = LOGSEVERITY_INFO;
    std::wstring cachePath = utils::getExePath() + L"\\cache";
    CefString(&settings.root_cache_path).FromWString(cachePath);
    CefString(&settings.cache_path).FromWString(cachePath);
    CefInitialize(main_args, settings, app, nullptr);
    LOG(INFO) << "cachePath=" << utils::ws2s(cachePath);
    CefRunMessageLoop();
    CefShutdown();
    return 0;
}
