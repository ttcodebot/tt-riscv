# KianV RV32E bare-metal SoC

Hirosh Dabui's [KianV RV32E bare-metal SoC](https://github.com/splinedrive/RISCV-KianV-BareMetalStyle)
(`tt_um_kianv_bare_metal`) is a small RV32E microcontroller: a multi-cycle KianV core without the M
extension or CSRs, code and data in the QSPI Pmod, a UART, an SPI master and PWM audio. It runs bare-metal
C built with the stock RISC-V GCC.

| Shuttle  | Project | Status                                                     |
| -------- | ------- | ---------------------------------------------------------- |
| tt07     | #588    | tested on silicon (RP2040 board, 25 MHz)                   |
| ttihp0p2 | #10     | should work, not verified (adds a flash setup-time fix)    |
| ttsky25b | #838    | should work, not verified (fork with ALU/register changes) |

## Hardware

- QSPI Pmod on the BIDIR header. Code runs from flash offset **1 MiB** (0x20100000); the 8 MiB PSRAM on
  `uio[6]` is at 0x80000000. The second PSRAM is not used.
- UART: TX on `uo_out[4]`, RX on `ui_in[3]` (the datasheet's pinout table lists the inputs wrongly).
- PWM audio (8-bit sigma-delta) on `uo_out[3]`; SPI master CS/SCK/MOSI on `uo_out[0..2]`, MISO on `ui_in[2]`.
- `uo_out[7:5]` show bits 18:16 of the program counter, so software can't drive them directly.
- Designed for 50 MHz; tested at 25 MHz.

| Address    | Register                                         |
| ---------- | ------------------------------------------------ |
| 0x10000000 | UART data (write: TX, read: RX, 16-byte FIFO)    |
| 0x10000005 | UART status: bit 0 RX ready, bit 5 TX empty      |
| 0x10000010 | UART divider (clock cycles per bit)              |
| 0x10000014 | PWM sample (8 bits) on `uo_out[3]`               |
| 0x10000018 | cycle counter                                    |
| 0x10500000 | SPI master (control/data, divider at 0x10500010) |

## Build the test program

```sh
sudo apt install gcc-riscv64-unknown-elf
make -C examples/kianv-rv32e                  # FREQ=25000000 BAUD=115200 by default
```

`kianv-rv32e-hello-25000000.bin` prints a banner, tests 64 KiB of PSRAM, counts 0-7 on `uo_out[7:5]`,
beeps on `uo_out[3]`, then answers single keys: `b` beep, `l` LED count, `m` PSRAM test, `c` cycle
counter, `h` help; other keys echo in upper case. `FREQ` must match the clock the SoC runs at.

## Flash and run

In the [Flasher](https://tinytapeout.github.io/tinytapeout-flasher/), use **Custom** at offset **1048576**.
This overwrites the KianV Linux bootloader (see [flash layout](flash-layout.md)).

```python
import uart_console
uart_console.kianv_rv32e()        # 25 MHz, 115200 baud
```

```
KianV RV32E bare-metal SoC: hello from silicon
build: FREQ=25000000 Hz, BAUD=115200
PSRAM test, 65536 bytes at 0x80100000: PASS
LED count 0..7 on uo_out[7:5]: 01234567 done
beep 1 kHz on uo_out[3] (PWM)
keys: b=beep  l=LED count  m=PSRAM test  c=cycle counter  h=help; others echo
>
```
