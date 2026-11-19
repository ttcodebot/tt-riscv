# RISC-V on Tiny Tapeout

[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0) ![AI Assisted](https://www.aihonestybadge.com/badges/ai-assisted-small.svg)

Run real software on the RISC-V CPUs taped out on [Tiny Tapeout](https://tinytapeout.com) chips:
Linux, Zephyr, MicroPython and Rust, with prebuilt images and step-by-step guides that have been tested
on silicon.

You need a Tiny Tapeout demo board with one of the chips below and a
[QSPI Pmod](https://store.tinytapeout.com/products/QSPI-Pmod-p716541602). Start with
[board setup](guides/board-setup.md).

## What runs where

| SoC                                            | Shuttle (project #)                                        | What runs                                | Guide                                                                                                                                                                         | Status                           |
| ---------------------------------------------- | ---------------------------------------------------------- | ---------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------- |
| KianV uLinux SoC (RV32IMA, 16 MiB PSRAM, GPIO) | ttsky25a (#846)                                            | Linux, Rust                              | [kianv-linux](guides/kianv-linux.md), [rust-kianv-linux](guides/rust-kianv-linux.md), [linux-apps](guides/linux-apps.md)                                                      | tested on silicon                |
|                                                | ttsky25a (#846)                                            | Zephyr + MicroPython                     | [kianv-zephyr](guides/kianv-zephyr.md)                                                                                                                                        | tested on silicon                |
|                                                | ttsky25a (#846)                                            | Memory self-test                         | [kianv-memtest](guides/kianv-memtest.md)                                                                                                                                      | tested on silicon                |
|                                                | ttsky25b (#910), ttihp25b (#522), ttcad25a (#974)          | same images                              | same guides                                                                                                                                                                   | should work, not verified        |
|                                                | ttgf0p2 (#295)                                             | Linux, Rust (same images, run at 30 MHz) | same guides                                                                                                                                                                   | tested on silicon                |
|                                                | ttgf0p3 (#518)                                             | same images                              | same guides                                                                                                                                                                   | tested on silicon                |
| KianV uLinux SoC, regfile edition              | ttsky25b (#970)                                            | Linux (same images)                      | [kianv-linux](guides/kianv-linux.md)                                                                                                                                          | tested on silicon                |
| KianV uLinux SoC (8 MiB, original)             | tt06 (#910)                                                | Linux; Zephyr + MicroPython              | Flasher preset "KianV uLinux Image (TT06)" + [kianv-linux](guides/kianv-linux.md) console steps; [kianv-zephyr](guides/kianv-zephyr.md)                                       | tested on silicon (RP2040 board) |
| KianV RV32E bare-metal SoC                     | tt07 (#588), ttihp0p2 (#10), ttsky25b (#838)               | bare-metal C                             | [kianv-rv32e](guides/kianv-rv32e.md)                                                                                                                                          | tested on silicon (tt07)         |
| TinyQV "Asteroids" / "Berzerk" (RV32EC)        | ttsky25a (#495, #687)                                      | MicroPython, Rust                        | [TinyQV Micropython](guides/tinyqv-micropython.md), [TinyQV Rust](guides/rust-tinyqv.md), [TinyQV C SDK](https://github.com/TinyTapeout/ttsky25a-tinyQV/blob/main/testing.md) | tested on silicon                |
| TinyQV (RV32EC)                                | ttihp25b (#490), ttihp25a, ttcad25a, ttsky25b, ttgf0p1-0p3 | MicroPython, C                           | [program.tinyqv.com](https://program.tinyqv.com)                                                                                                                              | see the programmer               |
| TinyQV (RV32EC, TT06 version)                  | tt06                                                       | MicroPython, C                           | [tt06-program.tinyqv.com](https://tt06-program.tinyqv.com)                                                                                                                    | see the programmer               |

Before switching between SoCs on one Pmod, read [flash layout](guides/flash-layout.md).

Experimental: [full Rust `std` on KianV Linux](guides/experimental/rust-std.md),
[Linux kernel executing in place from flash](guides/experimental/xip-kernel.md).

Many more RISC-V cores have been taped out on Tiny Tapeout, from SERV and nanoV to Wildcat, Goldcrest and
student designs. Upcoming shuttles bring a KianV with an MMU running mainline Linux (ttihp26a), a
Zephyr-targeted RV32IM SoC and PicoRV32 (ttsky26c). Their guides will be added here once the chips are
back. Each chip page on [tinytapeout.com](https://tinytapeout.com) links to its project.

## Images

Prebuilt images are published as [releases](../../releases). Each release lists the upstream commits
it was built from and the sha256 of every file:

| File                          | Flash offset (hex / decimal)    | Guide                                              |
| ----------------------------- | ------------------------------- | -------------------------------------------------- |
| `kianv-ulinux-bootloader.bin` | 0x100000 / 1048576              | [kianv-linux](guides/kianv-linux.md)               |
| `kianv-ulinux-kianv.dtb`      | 0x180000 / 1572864              |                                                    |
| `kianv-ulinux-Image`          | 0x200000 / 2097152              |                                                    |
| `kianv-ulinux-sdk.tar.gz`     | (not flashed: toolchain + QEMU) | [linux-apps](guides/linux-apps.md)                 |
| `kianv-memtest.bin`           | 0x100000 / 1048576              | [kianv-memtest](guides/kianv-memtest.md)           |
| `kianv-zephyr-dualboot.bin`   | 0x100000 / 1048576              | [kianv-zephyr](guides/kianv-zephyr.md)             |
| `tinyqv-micropython.bin`      | 0x000000 / 0                    | [tinyqv-micropython](guides/tinyqv-micropython.md) |
| `tinyqv-rust-hello.bin`       | 0x000000 / 0                    | [rust-tinyqv](guides/rust-tinyqv.md)               |

## Repository layout

| Path                                             | What                                                                                       |
| ------------------------------------------------ | ------------------------------------------------------------------------------------------ |
| [`guides/`](guides)                              | Step-by-step guides                                                                        |
| [`tools/uart_console.py`](tools/uart_console.py) | Console for KianV-style SoCs, run on the demo board                                        |
| [`pins.env`](pins.env)                           | Upstream commits every image is built from                                                 |
| [`linux/`](linux)                                | Programs built into the KianV Linux image (plain source + Buildroot packages), QEMU runner |
| [`patches/`](patches)                            | Changes on top of upstream, not yet merged there                                           |
| [`scripts/`](scripts)                            | Build scripts, also used by CI                                                             |
| [`examples/rust/`](examples/rust)                | Rust examples                                                                              |

To build images yourself, see [building](guides/building.md).

## Credits

- **KianV** RISC-V SoC, its Linux, Zephyr and MicroPython ports: Hirosh Dabui
  ([splinedrive/kianRiscV](https://github.com/splinedrive/kianRiscV)).
- **TinyQV**, its SDK, MicroPython port and programmer: Mike Bell ([MichaelBell](https://github.com/MichaelBell)),
  with peripherals by the TinyQV peripheral challenge contributors.
- **QSPI Pmod:** Leo Moser ([mole99/qspi-pmod](https://github.com/mole99/qspi-pmod)).

## License

Code, scripts and patches: [Apache-2.0](LICENSE). Guides and other documentation: [CC-BY-4.0](LICENSE-docs).
Patches to upstream projects are offered under those projects' licenses. Released images contain
software under their own licenses (Linux: GPL-2.0, Zephyr: Apache-2.0, MicroPython: MIT, ...).
