const statusElement = document.getElementById("status");
const changeCountElement = document.getElementById("change-count");
const statusIndicator = document.querySelector(".status-indicator");

let changeCount = 0;

function sendCommand(command) {
    if (command !== "close") {
        changeCount++;
        changeCountElement.textContent = changeCount.toString();
    }

    window.external.sendMessage(command);
}

function updateStatus(state) {
    const isTransparent = state === "transparent";

    statusElement.textContent = isTransparent
        ? "Transparent"
        : "Opaque";

    statusIndicator.style.background = isTransparent
        ? "#22c55e"
        : "#f59e0b";

    statusIndicator.style.boxShadow = isTransparent
        ? "0 0 18px rgb(34 197 94 / 80%)"
        : "0 0 18px rgb(245 158 11 / 80%)";
}

document.querySelectorAll("[data-command]").forEach(button => {
    button.addEventListener("click", () => {
        sendCommand(button.dataset.command);
    });
});

window.external.receiveMessage(updateStatus);