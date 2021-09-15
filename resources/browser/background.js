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

function is_target(element) {
    let tagList = ["DIV", "H1", "H2", "H3", "A", "INPUT", "SPAN", "P"];
    return tagList.indexOf(element.tagName) !== -1;
}

function has_child_target(element) {
    for (let child of element.children) {
        if (is_target(child) || has_child_target(child)) {
            return true;
        }
    }
    return false;
}

function is_keep_anyway(element) {
    let tagList = ["I"];
    return tagList.indexOf(element.tagName) !== -1;
}

function keep_element(element) {
    if (is_target(element)) {
        if (!has_child_target(element)) {
            return true;
        }
        if (element.hasChildNodes()) {
            for (let node of element.childNodes) {
                if (node.nodeType === 3 && node.textContent.trim().length > 0) {
                    console.log(element)
                    return true;
                }
            }
        }
    }
    if (is_keep_anyway(element)) {
        return true;
    }
    return false;
}

function get_all_rect(element, resArr) {
    for (let child of element.children) {
        let rect = child.getBoundingClientRect();
        if (rect.width > 0 && rect.height > 0 && keep_element(child)) {
            resArr.push([
                rect.left + window.scrollX,
                rect.top + window.scrollY,
                rect.width,
                rect.height
            ]);
        }
        else {
            get_all_rect(child, resArr);
        }
    }
}

async function full_page_tag(path) {
    log(`[TAG] ${document.readyState} ${document.URL}`);
    let style = document.createElement("style");
    style.innerHTML = `body::-webkit-scrollbar {display: none;}`;
    document.head.appendChild(style);
    let { width, height } = await get_full_page_size();
    await delay(500);
    let rects = [];
    get_all_rect(document.documentElement, rects);
    log(`get all ${rects.length} rect`);
    ipc_send("region_tag", {
        x: 0, y: 0, width, height,
        path, rects
    });
}

document.addEventListener('keydown', async (event) => {
    const keyName = event.key;
    if (keyName === 'F6') {
        await full_page_tag("");
        return;
    }
}, false);

window.chrome.webview.addEventListener("message", async (msg) => {
    try {
        log(`receive messge => ${JSON.stringify(msg.data)}`);
        let { channel, payload } = msg.data;
        switch (channel) {
            case "full_page_tag": await full_page_tag(payload["path"]); break;
        }
    } catch (e) {
        log(`[ERROR] ${e}`);
    }
});
