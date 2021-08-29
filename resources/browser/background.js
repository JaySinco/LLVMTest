function delay(ms) {
    return new Promise((resolve) => setTimeout(resolve, ms));
}

function scroll() {
    let element = document.scrollingElement;
    if (element === null) {
        log(`skip scroll`);
        return {
            width: document.body.clientWidth,
            height: document.body.clientHeight
        }
    }
    log(`scroll`);
    let width = element.scrollWidth;
    let height = element.scrollHeight;
    window.scrollTo(width, height);
    return { width, height };
}

function isFixed(e) {
    return e.style.position === 'fixed' ||
        window.getComputedStyle(e).getPropertyValue("position") === 'fixed';
}

function ipcSend(channel, payload) {
    window.chrome.webview.postMessage(JSON.stringify({ channel, payload }));
}

function log(msg) {
    ipcSend("log", msg);
}

async function main() {
    try {
        log(`[ENTER] ${document.readyState} ${document.URL}`);
        await delay(500);
        scroll();
        await delay(500);
        log(`change fixed element to static`);
        let elements = document.querySelectorAll('*');
        for (let e of elements) {
            if (isFixed(e)) {
                e.style.position = 'static';
            }
        }
        let { width, height } = scroll();
        await delay(500);
        log(`capture region ${width}x${height}`);
        ipcSend("capture", { width, height });
    }
    catch (e) {
        log(`[ERROR] ${e}`);
    }
}

main();
