#pragma once
#include "../utils.h"
#define GOOGLE_GLOG_DLL_DECL
#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>
#include <WebView2.h>
#include <wil/com.h>
#include <atomic>
#include <mutex>
#include <map>
#include <future>

class browser
{
public:
    browser();
    ~browser();
    bool open(const std::pair<int, int> &size = {1440, 900}, bool show = true);
    bool navigate(const std::wstring &url);
    bool screenshot(const std::wstring &path, int width, int height);
    bool call_devtools_protocol(const std::wstring &method, const std::wstring &paramsJson,
                                std::wstring &respJson);
    bool wait_utill_closed();
    bool is_closed() const;
    bool close();

private:
    using task_t = std::packaged_task<bool()>;
    enum class status
    {
        INITIAL,
        RUNNING,
        ABORT,
        CLOSED,
    };

    bool post_task(task_t &&task) const;
    HRESULT environment_created(HRESULT result, ICoreWebView2Environment *environment);
    HRESULT controller_created(HRESULT result, ICoreWebView2Controller *controller);
    HRESULT new_window_requested(ICoreWebView2 *sender,
                                 ICoreWebView2NewWindowRequestedEventArgs *args);
    HRESULT navigation_completed(ICoreWebView2 *sender,
                                 ICoreWebView2NavigationCompletedEventArgs *args);
    HRESULT web_message_received(ICoreWebView2 *sender,
                                 ICoreWebView2WebMessageReceivedEventArgs *args);
    LRESULT scoped_wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT wnd_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND h_browser = NULL;
    std::mutex lock_running;
    std::atomic<status> status_ = status::INITIAL;
    std::atomic<bool> navigate_completed = false;
    wil::com_ptr<ICoreWebView2Controller> wv_controller = nullptr;
    wil::com_ptr<ICoreWebView2> wv_window = nullptr;
};
