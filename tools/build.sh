#!/usr/bin/env bash

set -euo pipefail

root_directory="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
preset="${1:-}"

if [ -z "$preset" ]; then
    preset="debug"

    user_presets_path="$root_directory/CMakeUserPresets.json"

    if [ -f "$user_presets_path" ]; then
        local_package="$(
            python3 - "$user_presets_path" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as file:
    presets = json.load(file)

for preset in presets.get("configurePresets", []):
    if preset.get("name") == "debug-local":
        print(preset.get("cacheVariables", {}).get("PHOTINOX_NATIVE_PACKAGE", ""))
        break
PY
        )"

        if [ -n "$local_package" ]; then
            if [[ "$local_package" = /* ]]; then
                local_package_path="$local_package"
            else
                local_package_path="$root_directory/$local_package"
            fi

            if [ -f "$local_package_path" ]; then
                preset="debug-local"
                echo "Using local PhotinoX.Native package: $local_package_path"
            fi
        fi
    fi
fi

case "$preset" in
    debug|release|debug-local|release-local)
        ;;
    *)
        echo "Usage: $0 [debug|release|debug-local|release-local]" >&2
        exit 1
        ;;
esac

build_directory="${preset%-local}"
executable_path="$root_directory/build/$build_directory/samples/HelloWorld/PhotinoX.Cpp.HelloWorld"

cd "$root_directory"

cmake --preset "$preset"
cmake --build --preset "$preset"

"$executable_path"