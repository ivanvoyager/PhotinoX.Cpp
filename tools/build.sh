#!/usr/bin/env bash

set -euo pipefail

preset="${1:-debug}"
build_directory="${preset%-local}"

case "$preset" in
    debug|release|debug-local|release-local)
        ;;
    *)
        echo "Usage: $0 [debug|release]" >&2
        exit 1
        ;;
esac

root_directory="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
executable_path="$root_directory/build/$build_directory/samples/HelloWorld/PhotinoX.Cpp.HelloWorld"

cd "$root_directory"

cmake --preset "$preset"
cmake --build --preset "$preset"

"$executable_path"