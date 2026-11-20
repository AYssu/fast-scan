#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "$0")"
if [[ -n "${ANDROID_NDK:-}" ]]; then
    exec "$ANDROID_NDK/ndk-build" "$@"
fi
if command -v ndk-build >/dev/null 2>&1; then
    exec ndk-build "$@"
fi
printf '请设置 ANDROID_NDK 为 Android NDK 目录，或将 ndk-build 加入 PATH。\n' >&2
exit 1
