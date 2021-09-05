#include "browser.h"
#include <shlwapi.h>
#include <wrl.h>
#include <thread>
#include <fstream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#define WM_ASYNC_CALL WM_USER + 1

using namespace Microsoft::WRL;

browser::browser() {}

browser::~browser() { this->close(); }

bool browser::open(const std::pair<int, int> &size, bool show)
{
    std::thread([=] {
        std::lock_guard<std::mutex> lock(lock_running);
        WNDCLASS wc = {0};
        wc.lpszClassName = L"PROTOTYPING_BROWSER";
        wc.lpfnWndProc = wnd_proc;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        RegisterClass(&wc);
        h_browser = CreateWindow(wc.lpszClassName, L"browser", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                                 CW_USEDEFAULT, size.first, size.second, NULL, NULL,
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

bool browser::navigate_(const std::wstring &url)
{
    HRESULT hr = wv_window->Navigate(url.c_str());
    if (FAILED(hr)) {
        LOG(ERROR) << "failed to navigate to {}, hr={}"_format(utils::ws2s(url), hr);
        return false;
    }
    return true;
}

bool browser::navigate(const std::wstring &url)
{
    this->navigate_completed = false;
    if (!this->post_task_sync(std::bind(&browser::navigate_, this, std::cref(url)))) {
        return false;
    }
    while (!this->navigate_completed) {
        std::this_thread::sleep_for(100ms);
    }
    return true;
}

bool browser::full_page_tag_(const std::wstring &path)
{
    nlohmann::json payload = {
        {"path", utils::ws2s(path, true)},
    };
    if (!this->post_web_message_("full_page_tag", payload)) {
        return false;
    }
    return true;
}

bool browser::full_page_tag(const std::wstring &path)
{
    this->region_tag_completed = false;
    if (!this->post_task_sync(std::bind(&browser::full_page_tag_, this, std::cref(path)))) {
        return false;
    }
    while (!this->region_tag_completed) {
        std::this_thread::sleep_for(100ms);
    }
    return true;
}

bool browser::region_tag(const std::wstring &path, int width, int height,
                         const std::vector<cv::Rect> &rects)
{
    LOG(INFO) << "tag region {}x{} to {}"_format(width, height, utils::ws2s(path));
    std::shared_ptr<void> completed_guard(nullptr,
                                          [&](void *) { this->region_tag_completed = true; });
    nlohmann::json request;
    request["format"] = "png";
    request["fromSurface"] = true;
    request["captureBeyondViewport"] = true;
    request["clip"] = {
        {"x", 0}, {"y", 0}, {"width", width}, {"height", height}, {"scale", 1},
    };
    std::wstring respJson;
    if (!this->call_devtools_protocol(L"Page.captureScreenshot", utils::s2ws(request.dump(), true),
                                      respJson)) {
        return false;
    }
    auto response = nlohmann::json::parse(utils::ws2s(respJson, true));
    auto data = utils::base64_decode(response["data"].get<std::string>());
    auto image = cv::imdecode(cv::Mat(data), cv::IMREAD_COLOR);
    for (const auto rect: rects) {
        cv::rectangle(image, rect, {0, 0, 0}, 3);
    }
    cv::imwrite(utils::ws2s(path), image);
    LOG(INFO) << "tag done";
    return true;
}

bool browser::call_devtools_protocol_(const std::wstring &method, const std::wstring &paramsJson,
                                      std::promise<std::optional<std::wstring>> *resp)
{
    HRESULT hr = wv_window->CallDevToolsProtocolMethod(
        method.c_str(), paramsJson.c_str(),
        Callback<ICoreWebView2CallDevToolsProtocolMethodCompletedHandler>([=](HRESULT error,
                                                                              LPCWSTR resultJson)
                                                                              -> HRESULT {
            if (FAILED(error)) {
                LOG(ERROR) << "error occurred when calling protocol {}, hr={}"_format(
                    utils::ws2s(method), error);
                if (resp) resp->set_value({});
            } else {
                if (resp) resp->set_value(resultJson);
            }
            return S_OK;
        }).Get());
    if (FAILED(hr)) {
        LOG(ERROR) << "failed to call protocol {}, hr={}"_format(utils::ws2s(method), hr);
        return false;
    }
    return true;
}

bool browser::post_web_message_(const std::string &channel, const nlohmann::json &payload)
{
    nlohmann::json msgJSON = {
        {"channel", channel},
        {"payload", payload},
    };
    std::wstring msg = utils::s2ws(msgJSON.dump(), true);
    HRESULT hr = wv_window->PostWebMessageAsJson(msg.c_str());
    if (FAILED(hr)) {
        LOG(ERROR) << "failed to post web message {}, hr={}"_format(msgJSON.dump(), hr);
        return false;
    }
    return true;
}

bool browser::call_devtools_protocol(const std::wstring &method, const std::wstring &paramsJson,
                                     std::wstring &respJson)
{
    std::promise<std::optional<std::wstring>> resp;
    auto resp_fut = resp.get_future();
    if (!this->post_task_sync(std::bind(&browser::call_devtools_protocol_, this, std::cref(method),
                                        std::cref(paramsJson), &resp))) {
        return false;
    }
    if (auto j = resp_fut.get()) {
        respJson = *j;
        return true;
    }
    return false;
}

bool browser::execute_script_(const std::wstring &script,
                              std::promise<std::optional<std::wstring>> *resp)
{
    HRESULT hr = wv_window->ExecuteScript(
        script.c_str(),
        Callback<ICoreWebView2ExecuteScriptCompletedHandler>([=](HRESULT error,
                                                                 LPCWSTR resultJson) -> HRESULT {
            if (FAILED(error)) {
                LOG(ERROR) << "error occurred when executing script, hr=" << error;
                if (resp) resp->set_value({});
            } else {
                if (resp) resp->set_value(resultJson);
            }
            return S_OK;
        }).Get());
    if (FAILED(hr)) {
        LOG(ERROR) << "failed to execute script, hr=" << hr;
        return false;
    }
    return true;
}

bool browser::execute_script(const std::wstring &script, std::wstring &respJson)
{
    std::promise<std::optional<std::wstring>> resp;
    auto resp_fut = resp.get_future();
    if (!this->post_task_sync(
            std::bind(&browser::execute_script_, this, std::cref(script), &resp))) {
        return false;
    }
    if (auto j = resp_fut.get()) {
        respJson = *j;
        return true;
    }
    return false;
}

bool browser::post_task_sync(std::function<bool()> task) const
{
    if (is_closed()) {
        LOG(ERROR) << "failed to post task, browser is closed";
        return false;
    }
    auto ptask = new std::packaged_task<bool()>(std::move(task));
    auto future = ptask->get_future();
    BOOL ok = PostMessage(h_browser, WM_ASYNC_CALL, reinterpret_cast<WPARAM>(ptask), NULL);
    if (!ok) {
        LOG(ERROR) << "failed to post message";
        return false;
    }
    return future.get();
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
    // wv_window->OpenDevToolsWindow();

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
    auto stat = utils::readFile(L"resources/browser/background.js");
    if (!stat.first) {
        LOG(ERROR) << "failed to load background.js";
        return S_OK;
    }
    std::wstring source = utils::s2ws(stat.second, true);
    std::wstring respJson;
    this->execute_script_(source);
    return S_OK;
}

HRESULT browser::web_message_received(ICoreWebView2 *sender,
                                      ICoreWebView2WebMessageReceivedEventArgs *args)
{
    wchar_t *msg;
    args->TryGetWebMessageAsString(&msg);
    std::shared_ptr<void> msg_guard(nullptr, [&](void *) { CoTaskMemFree(msg); });
    std::string msgJson = utils::ws2s(msg, true);
    auto message = nlohmann::json::parse(msgJson);
    std::string channel = message["channel"].get<std::string>();
    auto payload = message["payload"];
    if (channel == "log") {
        LOG(INFO) << payload.get<std::string>();
    } else if (channel == "region_tag") {
        std::wstring path = utils::s2ws(payload["path"].get<std::string>(), true);
        if (path.size() <= 0) {
            path = utils::getExeDir() + L"\\screenshot.png";
        }
        int width = payload["width"].get<int>();
        int height = payload["height"].get<int>();
        std::thread([=]() { this->region_tag(path, width, height); }).detach();
    }
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
            using task_t = std::packaged_task<bool()>;
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
