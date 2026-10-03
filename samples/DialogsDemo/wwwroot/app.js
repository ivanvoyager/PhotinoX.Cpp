const resultElement = document.getElementById("result");
const commandButtons = [...document.querySelectorAll("[data-command]")];
const clearButton = document.getElementById("clear-result");

function setBusy(busy) {
    commandButtons.forEach(button => {
        button.disabled = busy;
    });
}

function showResult(message) {
    if (message.type === "dialog-result") {
        const paths = message.paths.length > 0
            ? message.paths.map(path => `• ${path}`).join("\n")
            : "The dialog was canceled.";

        resultElement.textContent = `${message.title}\n\n${paths}`;
        return;
    }

    if (message.type === "message-result") {
        resultElement.textContent = `Message dialog result\n\n${message.result}`;
        return;
    }

    if (message.type === "error") {
        resultElement.textContent = `Error\n\n${message.message}`;
    }
}

commandButtons.forEach(button => {
    button.addEventListener("click", () => {
        setBusy(true);
        window.external.sendMessage(button.dataset.command);
    });
});

clearButton.addEventListener("click", () => {
    resultElement.textContent = "Choose a dialog operation.";
});

window.external.receiveMessage(message => {
    try {
        showResult(JSON.parse(message));
    } finally {
        setBusy(false);
    }
});
``