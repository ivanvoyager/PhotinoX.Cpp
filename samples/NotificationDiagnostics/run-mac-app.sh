#!/usr/bin/env bash
set -euo pipefail

# Builds and runs the NotificationDiagnostics sample as a macOS .app bundle.
#
# Why this is needed:
# macOS native notifications require a real .app bundle identity.
# Running the sample directly from the CMake output directory may start the app,
# but UNUserNotificationCenter cannot be initialized from an unbundled executable.
#
# Usage from the repository root:
#   chmod +x samples/NotificationDiagnostics/run-mac-app.sh
#   samples/NotificationDiagnostics/run-mac-app.sh
#
# Optional arguments:
#   samples/NotificationDiagnostics/run-mac-app.sh Debug
#   samples/NotificationDiagnostics/run-mac-app.sh Release
#
# Usage from the sample directory:
#   chmod +x run-mac-app.sh
#   ./run-mac-app.sh
#   ./run-mac-app.sh Debug
#   ./run-mac-app.sh Release
#
# By default, the script starts the generated .app through LaunchServices
# and keeps stdout/stderr attached to the current terminal.
#
# To launch detached, like a regular double-click/open:
#   PHOTINOX_DETACHED_OPEN=1 ./run-mac-app.sh
#
# The script:
#   1. Builds the sample if the executable is missing.
#   2. Creates PhotinoX.Cpp.NotificationDiagnostics.app under the CMake sample output directory.
#   3. Copies the full build output into Contents/MacOS.
#   4. Writes a minimal Info.plist with a stable CFBundleIdentifier.
#   5. Starts the app through LaunchServices using `open`.

CONFIGURATION="${1:-Debug}"
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPOSITORY_DIR="$(cd "$PROJECT_DIR/../.." && pwd)"
BUILD_DIR="$REPOSITORY_DIR/build/$(printf '%s' "$CONFIGURATION" | tr '[:upper:]' '[:lower:]')"
OUTPUT_DIR="$BUILD_DIR/samples/NotificationDiagnostics"
APP_NAME="PhotinoX.Cpp.NotificationDiagnostics"
BUNDLE_ID="io.photinox.cpp.NotificationDiagnostics"

APP_DIR="$OUTPUT_DIR/$APP_NAME.app"
CONTENTS_DIR="$APP_DIR/Contents"
MACOS_DIR="$CONTENTS_DIR/MacOS"
RESOURCES_DIR="$CONTENTS_DIR/Resources"

EXECUTABLE="$OUTPUT_DIR/$APP_NAME"

if [ ! -f "$EXECUTABLE" ]; then
    cmake \
        -S "$REPOSITORY_DIR" \
        -B "$BUILD_DIR" \
        -DCMAKE_BUILD_TYPE="$CONFIGURATION" \
        -DPHOTINOX_BUILD_SAMPLES=ON

    cmake \
        --build "$BUILD_DIR" \
        --target "$APP_NAME" \
        --config "$CONFIGURATION"
fi

rm -rf "$APP_DIR"
mkdir -p "$MACOS_DIR" "$RESOURCES_DIR"

cat > "$CONTENTS_DIR/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "https://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
  <dict>
    <key>CFBundleExecutable</key>
    <string>$APP_NAME</string>

    <key>CFBundleIdentifier</key>
    <string>$BUNDLE_ID</string>

    <key>CFBundleName</key>
    <string>Notification Diagnostics</string>

    <key>CFBundleDisplayName</key>
    <string>Notification Diagnostics</string>

    <key>CFBundlePackageType</key>
    <string>APPL</string>

    <key>CFBundleVersion</key>
    <string>1</string>

    <key>CFBundleShortVersionString</key>
    <string>1.0</string>

    <key>LSMinimumSystemVersion</key>
    <string>11.0</string>
  </dict>
</plist>
PLIST

cp "$EXECUTABLE" "$MACOS_DIR/"
cp "$OUTPUT_DIR/PhotinoX.Native.dylib" "$MACOS_DIR/"
cp -R "$OUTPUT_DIR/wwwroot" "$MACOS_DIR/"
cp -R "$OUTPUT_DIR/assets" "$MACOS_DIR/"

chmod +x "$MACOS_DIR/$APP_NAME"

echo "Signing $APP_NAME.app with ad-hoc signature..."
codesign --force --deep --sign - "$APP_DIR"

echo "Verifying code signature..."
codesign --verify --deep --strict --verbose=2 "$APP_DIR"

echo "Code signature details:"
codesign -dv --verbose=4 "$APP_DIR" 2>&1 || true

echo "Registering $APP_NAME.app with LaunchServices..."
/System/Library/Frameworks/CoreServices.framework/Frameworks/LaunchServices.framework/Support/lsregister \
    -f "$APP_DIR"

cleanup() {
    echo "Stopping $APP_NAME..."
    pkill -f "$MACOS_DIR/$APP_NAME" 2>/dev/null || true
}

trap cleanup EXIT INT TERM

if [ "${PHOTINOX_DETACHED_OPEN:-0}" = "1" ]; then
    echo "Starting $APP_NAME.app through LaunchServices without attached stdout/stderr..."
    open "$APP_DIR"
else
    echo "Starting $APP_NAME.app through LaunchServices with stdout/stderr attached..."
    open -W --stdout "$(tty)" --stderr "$(tty)" "$APP_DIR"
fi
