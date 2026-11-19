#!/usr/bin/env bash
# Build the KianV Zephyr + MicroPython dual-boot image (Docker required, like the upstream port).
# Usage: scripts/build-zephyr.sh [out-dir]   (default: out/kianv-zephyr)
set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
. "$ROOT/pins.env"
OUT=${1:-$ROOT/out/kianv-zephyr}
mkdir -p "$OUT" && OUT=$(cd "$OUT" && pwd)  # absolute: the script changes directory
SRC=$ROOT/build/kianRiscV
"$ROOT/scripts/fetch-kianriscv.sh" "$SRC"
cd "$SRC/asic/os/zephyr_kianv_asic"
# Pin Zephyr: create the west workspace at the pinned commit; the port's Makefile reuses it.
if [ ! -d zephyrproject/.west ]; then
  # Host-side west in its own venv: the port's Docker build creates .venv inside the container.
  python3 -m venv "$ROOT/build/west-venv" && "$ROOT/build/west-venv/bin/pip" -q install west
  "$ROOT/build/west-venv/bin/west" init -m https://github.com/zephyrproject-rtos/zephyr --mr main zephyrproject
fi
git -C zephyrproject/zephyr fetch -q origin "$ZEPHYR_COMMIT"
git -C zephyrproject/zephyr -c advice.detachedHead=false checkout -q "$ZEPHYR_COMMIT"
# DOCKER_IMAGE_TAR (CI): reuse the port's Docker image from that file, or build it once and save it
# there. Building it downloads ~340 MB of Ubuntu packages, which dominates the build when the mirror
# is slow. `make -o docker-image` then skips the image step without touching the port's Makefile.
skip_image=
if [ -n "${DOCKER_IMAGE_TAR:-}" ]; then
  image=$(sed -n 's/^DOCKER_IMAGE ?= //p' Makefile)
  if [ -f "$DOCKER_IMAGE_TAR" ]; then
    docker load -i "$DOCKER_IMAGE_TAR"
  else
    make docker-image
    mkdir -p "$(dirname "$DOCKER_IMAGE_TAR")"
    docker save "$image" -o "$DOCKER_IMAGE_TAR"
  fi
  skip_image="-o docker-image"
fi
# shellcheck disable=SC2086
make $skip_image CPU_FREQ="$KIANV_CPU_FREQ" BAUDRATE="$KIANV_BAUDRATE"
B=build-docker
mkdir -p "$OUT"
cp $B/firmware_dualboot_nor_payload.bin $B/firmware_dualboot_nor_8m.bin $B/boot/bootloader_dualboot.bin \
   $B/zephyr/zephyr.bin $B/zephyr/zephyr.elf "$OUT/"
cp micropython/ports/kianv_riscv/build/firmware.bin "$OUT/micropython.bin"
{
  echo "zephyr: $(git -C zephyrproject/zephyr rev-parse HEAD)"
  echo "micropython: $(git -C micropython rev-parse HEAD)"
} > "$OUT/VERSIONS.txt"
(cd "$OUT" && sha256sum ./*.bin ./*.elf > SHA256SUMS && ls -l)
