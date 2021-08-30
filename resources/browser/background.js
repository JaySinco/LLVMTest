function delay(ms) {
    return new Promise((resolve) => setTimeout(resolve, ms));
}

function ipc_send(channel, payload) {
    window.chrome.webview.postMessage(JSON.stringify({ channel, payload }));
}

function log(msg) {
    ipc_send("log", `[WEB] ${msg}`);
}

function get_viewport() {
    let element = document.documentElement;
    if (document.documentElement === null) {
        element = document.body;
    };
    return {
        width: element.clientWidth,
        height: element.clientHeight
    };
}

async function scroll() {
    await delay(500);
    let root = document.scrollingElement;
    let hasScrollBar = false;
    let width = root.clientWidth;
    if (root.clientWidth < root.scrollWidth) {
        log("has horizontal scroll bar");
        hasScrollBar = true;
        width = root.scrollWidth;
    }
    let height = root.clientHeight;
    if (root.clientHeight < root.scrollHeight) {
        log("has vertical scroll bar");
        hasScrollBar = true;
        height = root.scrollHeight;
    }
    if (hasScrollBar) {
        log(`scroll`);
        window.scrollTo(width, height);
    }
    else {
        log(`skip scroll`);
    }
    return { width, height };
}

function is_fixed(e) {
    return e.style.position === 'fixed' ||
        window.getComputedStyle(e).getPropertyValue("position") === 'fixed';
}

async function get_full_page_size() {
    await scroll();
    await scroll();
    log(`change fixed element to static`);
    let elements = document.querySelectorAll('*');
    for (let e of elements) {
        if (is_fixed(e)) {
            e.style.position = 'static';
        }
    }
    let { width, height } = await scroll();
    return { width, height };
}

async function full_page_screenshot(path) {
    log(`[SCREENSHOT] ${document.readyState} ${document.URL}`);
    let { width, height } = await get_full_page_size();
    await delay(500);
    ipc_send("region_screenshot", { width, height, path });
}

document.addEventListener('keydown', async (event) => {
    const keyName = event.key;
    if (keyName === 'F6') {
        await full_page_screenshot("");
        return;
    }
}, false);

window.chrome.webview.addEventListener("message", async (msg) => {
    try {
        log(`receive messge => ${JSON.stringify(msg.data)}`);
        let { channel, payload } = msg.data;
        switch (channel) {
            case "full_page_screenshot": await full_page_screenshot(payload["path"]); break;
        }
    } catch (e) {
        log(`[ERROR] ${e}`);
    }
});
