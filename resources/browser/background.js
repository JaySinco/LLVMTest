function delay(ms) {
    return new Promise((resolve) => setTimeout(resolve, ms));
}

function scroll() {
    let width = document.documentElement.scrollWidth;
    let height = document.documentElement.scrollHeight;
    window.scrollTo(width, height);
    return { width, height };
}

async function main() {
    scroll();
    await delay(500);
    let { width, height } = scroll();
    await delay(500);
    window.chrome.webview.postMessage(JSON.stringify({
        scrollWidth: width,
        scrollHeight: height,
    }));
}

main();
