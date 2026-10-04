(function () {
    const hostList = document.getElementById("host-list");
    const photinoxList = document.getElementById("photinox-list");
    const platformList = document.getElementById("platform-list");

    function post(message) {
        if (window.external && typeof window.external.sendMessage === "function") {
            window.external.sendMessage(message);
            return;
        }

        renderError("Photino host bridge is not available.");
    }

    function render(info) {
        const platformName = normalizeValue(get(info, "Platform"));

        renderRows(hostList, [
            ["Platform", platformName],
            ["Process architecture", get(info, "ProcessArchitecture")],
            ["Language runtime", get(info, "LanguageRuntime")]
        ]);

        renderRows(photinoxList, [
            ["Native", get(info, "NativeVersion")],
            ["Engine", get(info, "WebViewEngine")],
            ["WebView", get(info, "WebViewRuntimeVersion")]
        ]);

        const platformRows = getPlatformRows(info, platformName);
        const platformSection = document.querySelector(".platform-section");

        if (platformRows.length === 0) {
            platformSection.hidden = true;
        } else {
            platformSection.hidden = false;
            renderRows(platformList, platformRows);
        }
    }

    function getPlatformRows(info, platformName) {
        if (platformName === "Windows") {
            const windows = get(info, "Windows") || {};

            return compactRows([
                ["WebView2 runtime", get(windows, "WebView2RuntimeVersion")]
            ]);
        }

        if (platformName === "Linux") {
            const linux = get(info, "Linux") || {};

            return compactRows([
                ["glibc", get(linux, "GlibcVersion")],
                ["GTK", get(linux, "GtkVersion")],
                ["WebKitGTK API", get(linux, "WebKitGtkApiTarget")],
                ["WebKitGTK runtime", get(linux, "WebKitGtkRuntimeVersion")]
            ]);
        }

        if (platformName === "MacOS") {
            const macOS = get(info, "MacOS") || {};

            return compactRows([
                ["WebKit", get(macOS, "WebKitVersion")]
            ]);
        }

        return [];
    }

    function compactRows(rows) {
        return rows.filter(([, value]) =>
            value !== null &&
            value !== undefined &&
            value !== "");
    }

    function renderRows(container, rows) {
        container.innerHTML = rows
            .map(([name, value]) => {
                const text = normalizeValue(value);

                return `
                    <div>
                      <dt>${escapeHtml(name)}</dt>
                      <dd title="${escapeHtml(text)}">${escapeHtml(text)}</dd>
                    </div>
                `;
            })
            .join("");
    }

    function renderError(message) {
        renderRows(hostList, [["Error", message]]);
        renderRows(photinoxList, []);
        renderRows(platformList, []);
    }

    function get(source, name) {
        if (!source)
            return null;

        if (Object.prototype.hasOwnProperty.call(source, name))
            return source[name];

        const camelName = name.charAt(0).toLowerCase() + name.slice(1);

        if (Object.prototype.hasOwnProperty.call(source, camelName))
            return source[camelName];

        return null;
    }

    function normalizeValue(value) {
        if (value === null || value === undefined || value === "")
            return "-";

        return String(value);
    }

    function escapeHtml(value) {
        return String(value ?? "-")
            .replaceAll("&", "&amp;")
            .replaceAll("<", "&lt;")
            .replaceAll(">", "&gt;")
            .replaceAll('"', "&quot;")
            .replaceAll("'", "&#39;");
    }

    if (window.external && typeof window.external.receiveMessage === "function") {
        window.external.receiveMessage(function (message) {
            try {
                render(JSON.parse(message));
            } catch (error) {
                console.error("Invalid runtime info message.", error, message);
                renderError("Invalid runtime info message.");
            }
        });
    }

    post("getRuntimeInfo");
})();