# CustomSchemes

Sample for loading HTML, CSS, JavaScript, and JSON through a custom `app://` scheme.

Expected behavior:

- `app://localhost/index.html` loads as the main document.
- `app://localhost/style.css` loads as a stylesheet.
- `app://localhost/app.js` loads as a script.
- `fetch("app://localhost/data.json")` succeeds.
- The native custom scheme handler is invoked for each custom scheme resource.