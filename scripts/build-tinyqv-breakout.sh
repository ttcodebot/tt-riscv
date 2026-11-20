#!/usr/bin/env bash
# Build examples/tinyqv-breakout (flash image for offset 0) with the tinyQV toolchain and SDK
# pinned in pins.env. The toolchain needs glibc 2.38 or newer (e.g. Ubuntu 24.04).
# Usage: scripts/build-tinyqv-breakout.sh [out-dir]   (default: out/tinyqv-breakout)
# Env:   RISCV_TOOLCHAIN  an installed tinyQV toolchain to use instead of downloading one
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
. "$ROOT/pins.env"
OUT=${1:-$ROOT/out/tinyqv-breakout}
mkdir -p "$OUT" && OUT=$(cd "$OUT" && pwd)
B=$ROOT/build
mkdir -p "$B"
if [ -z "${RISCV_TOOLCHAIN:-}" ]; then
  RISCV_TOOLCHAIN=$B/tinyqv-toolchain/tinyQV
  if [ ! -x "$RISCV_TOOLCHAIN/bin/riscv32-unknown-elf-gcc" ]; then
    tgz=$B/$(basename "$TINYQV_TOOLCHAIN_URL")
    [ -f "$tgz" ] || wget -q --tries=5 --retry-on-http-error=429,503 -O "$tgz" "$TINYQV_TOOLCHAIN_URL"
    echo "$TINYQV_TOOLCHAIN_SHA256  $tgz" | sha256sum -c -
    rm -rf "$B/tinyqv-toolchain" && mkdir -p "$B/tinyqv-toolchain"
    tar -xzf "$tgz" -C "$B/tinyqv-toolchain"
  fi
fi
export RISCV_TOOLCHAIN
SDK=$B/tinyQV-sdk
if [ "$(git -C "$SDK" rev-parse HEAD 2>/dev/null)" != "$TINYQV_SDK_COMMIT" ]; then
  rm -rf "$SDK"
  git init -q "$SDK"
  git -C "$SDK" fetch -q --depth 1 "$TINYQV_SDK_REPO" "$TINYQV_SDK_COMMIT"
  git -C "$SDK" -c advice.detachedHead=false checkout -q FETCH_HEAD
fi
# The SDK Makefile finds its headers through $(PWD): run it from the SDK directory.
(cd "$SDK" && make tinyQV.a tinyQV-asteroids.a start.o)
make -C "$ROOT/examples/tinyqv-breakout" clean all TINYQV_SDK="$SDK"
cp "$ROOT/examples/tinyqv-breakout/breakout.bin" "$OUT/tinyqv-breakout.bin"
(cd "$OUT" && sha256sum tinyqv-breakout.bin > SHA256SUMS && ls -l)
