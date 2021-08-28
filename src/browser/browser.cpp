#include "browser.h"
#include <wrl.h>
#include <thread>
#define WM_ASYNC_CALL WM_USER + 1

using namespace Microsoft::WRL;

browser::browser() {}

browser::~browser() { this->close(); }

bool browser::open(const std::wstring &title, const std::pair<int, int> &size, bool show)
{
    std::thread([=] {
        std::lock_guard<std::mutex> lock(lock_running);
        WNDCLASS wc = {0};
        wc.lpszClassName = L"PROTOTYPING_BROWSER";
        wc.lpfnWndProc = wnd_proc;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        RegisterClass(&wc);
        h_browser = CreateWindow(wc.lpszClassName, title.c_str(), WS_OVERLAPPEDWINDOW,
                                 CW_USEDEFAULT, CW_USEDEFAULT, size.first, size.second, NULL, NULL,
                                 GetModuleHandle(NULL), this);
        if (!h_browser) {
            LOG(ERROR) << "failed to create window";
            status_ = status::ABORT;
            return;
        }
        ShowWindow(h_browser, show);
        HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(
            nullptr, L"cache", nullptr,
            Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
                this, &browser::environment_created)
                .Get());
        if (FAILED(hr)) {
            LOG(ERROR) << "failed to create webview environment, hr=" << hr;
            status_ = status::ABORT;
            return;
        }
        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        status_ = status::CLOSED;
    }).detach();

    while (status_ == status::INITIAL) {
        std::this_thread::sleep_for(10ms);
    }
    if (status_ == status::RUNNING) {
        return true;
    }
    return false;
}

bool browser::close()
{
    if (!this->is_closed()) {
        PostMessage(h_browser, WM_DESTROY, NULL, NULL);
        this->wait_utill_closed();
    }
    return true;
}

bool browser::wait_utill_closed()
{
    std::lock_guard<std::mutex> lock(lock_running);
    return true;
}

bool browser::is_closed() const { return status_ == status::CLOSED; }

bool browser::navigate(const std::wstring &url)
{
    this->navigate_completed = false;
    task_t task([=]() -> data_t {
        HRESULT hr = wv_window->Navigate(url.c_str());
        if (FAILED(hr)) {
            return {{"err", "failed to navigate to {}, hr={}"_format(utils::ws2s(url), hr)}};
        }
        return {};
    });
    auto future = task.get_future();
    this->post_task(std::move(task));
    auto data = future.get();
    if (data.find("err") != data.end()) {
        LOG(ERROR) << data["err"];
        return false;
    }
    while (!this->navigate_completed) {
        std::this_thread::sleep_for(100ms);
    }
    return true;
}

std::pair<bool, std::string> browser::run_script(const std::wstring &source)
{
    std::promise<data_t> promise1;
    auto future1 = promise1.get_future();
    task_t task([=, &promise1]() -> data_t {
        HRESULT hr = wv_window->ExecuteScript(
            source.c_str(),
            Callback<ICoreWebView2ExecuteScriptCompletedHandler>([&](HRESULT err_code,
                                                                     LPCWSTR res_json) -> HRESULT {
                data_t data1;
                if (FAILED(err_code)) {
                    data1["err"] = "failed to execute script, hr={}"_format(err_code);
                } else {
                    data1["obj"] = utils::ws2s(res_json, true);
                }
                promise1.set_value(data1);
                return S_OK;
            }).Get());
        if (FAILED(hr)) {
            return {{"err", "failed to execute script, hr={}"_format(hr)}};
        }
        return {};
    });
    auto future2 = task.get_future();
    this->post_task(std::move(task));
    auto data2 = future2.get();
    if (data2.find("err") != data2.end()) {
        LOG(ERROR) << data2["err"];
        return {false, ""};
    }
    auto data1 = future1.get();
    if (data1.find("err") != data1.end()) {
        LOG(ERROR) << data1["err"];
        return {false, ""};
    }
    return {true, data1["obj"]};
}

bool browser::post_task(task_t &&task) const
{
    if (is_closed()) {
        LOG(ERROR) << "failed to post task: browser is closed";
        return false;
    }
    task_t *p_task = new task_t(std::move(task));
    BOOL ok = PostMessage(h_browser, WM_ASYNC_CALL, reinterpret_cast<WPARAM>(p_task), NULL);
    if (!ok) {
        LOG(ERROR) << "failed to post message";
        return false;
    }
    return true;
}

HRESULT browser::environment_created(HRESULT result, ICoreWebView2Environment *environment)
{
    return environment->CreateCoreWebView2Controller(
        h_browser, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                       this, &browser::controller_created)
                       .Get());
}

HRESULT browser::controller_created(HRESULT result, ICoreWebView2Controller *controller)
{
    if (controller != nullptr) {
        wv_controller = controller;
        wv_controller->get_CoreWebView2(&wv_window);
    }
    ICoreWebView2Settings *config;
    wv_window->get_Settings(&config);
    config->put_IsScriptEnabled(TRUE);
    config->put_AreDefaultScriptDialogsEnabled(TRUE);
    config->put_IsWebMessageEnabled(TRUE);
    config->put_AreDevToolsEnabled(TRUE);
    RECT rect;
    GetClientRect(h_browser, &rect);
    wv_controller->put_Bounds(rect);
    wv_window->OpenDevToolsWindow();

    wv_window->add_NewWindowRequested(
        Callback<ICoreWebView2NewWindowRequestedEventHandler>(this, &browser::new_window_requested)
            .Get(),
        nullptr);

    wv_window->add_NavigationCompleted(
        Callback<ICoreWebView2NavigationCompletedEventHandler>(this, &browser::navigation_completed)
            .Get(),
        nullptr);

    wv_window->add_WebMessageReceived(
        Callback<ICoreWebView2WebMessageReceivedEventHandler>(this, &browser::web_message_received)
            .Get(),
        nullptr);

    status_ = status::RUNNING;
    return S_OK;
}

HRESULT browser::new_window_requested(ICoreWebView2 *sender,
                                      ICoreWebView2NewWindowRequestedEventArgs *args)
{
    wchar_t *url;
    args->get_Uri(&url);
    sender->Navigate(url);
    CoTaskMemFree(url);
    args->put_Handled(TRUE);
    return S_OK;
}

HRESULT browser::navigation_completed(ICoreWebView2 *sender,
                                      ICoreWebView2NavigationCompletedEventArgs *args)
{
    this->navigate_completed = true;
    return S_OK;
}

HRESULT browser::web_message_received(ICoreWebView2 *sender,
                                      ICoreWebView2WebMessageReceivedEventArgs *args)
{
    wchar_t *msg;
    args->TryGetWebMessageAsString(&msg);
    LOG(INFO) << "[MESSAGE] " << utils::ws2s(msg);
    CoTaskMemFree(msg);
    return S_OK;
}

LRESULT CALLBACK browser::scoped_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        case WM_SIZE:
            if (wv_controller) {
                RECT rect;
                GetClientRect(hwnd, &rect);
                wv_controller->put_Bounds(rect);
            };
            break;
        case WM_DESTROY:
            VLOG(1) << "clean up webview2";
            wv_controller = nullptr;
            wv_window = nullptr;
            PostQuitMessage(0);
            return 0;
        case WM_ASYNC_CALL: {
            auto task = std::unique_ptr<task_t>(reinterpret_cast<task_t *>(wParam));
            (*task)();
            return 0;
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK browser::wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (msg == WM_NCCREATE) {
        CREATESTRUCTW *create = reinterpret_cast<CREATESTRUCTW *>(lParam);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(create->lpCreateParams));
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    auto br = reinterpret_cast<browser *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    return br->scoped_wnd_proc(hwnd, msg, wParam, lParam);
}
