# Flash layout: switching between SoCs

The QSPI Pmod flash is shared by every SoC on the chip. Each SoC starts executing at a fixed address,
so the images overlap:

| Image                                  | Flash range                   | Starts at |
| -------------------------------------- | ----------------------------- | --------- |
| TinyQV programs (MicroPython, Rust, C) | 0x000000 - size               | 0x000000  |
| TinyQV MicroPython (ttsky25a)          | 0x000000 - 0x13C998 (1.27 MB) | 0x000000  |
| KianV bootloader or memtest            | 0x100000 - 0x101FFF           | 0x100000  |
| KianV device tree                      | 0x180000                      |           |
| KianV Linux Image                      | 0x200000 - about 0x540000     |           |
| KianV Zephyr + MicroPython dual-boot   | 0x100000 - 0x3B1000           | 0x100000  |
| KianV RV32E bare-metal programs        | 0x100000 - size               | 0x100000  |

Rules:

- **TinyQV MicroPython and KianV Linux do not fit together.** MicroPython runs past 0x100000 and
  overwrites the KianV bootloader. Reflash whichever you switch to.
- Small TinyQV programs (below 1 MiB, e.g. the Rust example) can stay next to KianV Linux.
- The KianV bootloader copies a fixed number of bytes, set when it is built. Always flash the
  `bootloader.bin` and `Image` from the **same release**.
- Zephyr and Linux overwrite each other. Reflash when you switch.
- After running TinyQV, the PSRAMs are left in quad (QPI) mode. KianV then fails in odd ways (for
  example, the memtest hangs after its banner). `uart_console.py` resets the PSRAMs before starting
  KianV. If you start KianV some other way, power-cycle the board first.
