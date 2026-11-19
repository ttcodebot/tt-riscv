# SPDX-License-Identifier: Apache-2.0
"""UART console for RISC-V SoCs on Tiny Tapeout, run on the demo board (TT MicroPython SDK).

Copy it to the board once:      mpremote cp tools/uart_console.py :
Then, at the board's REPL:      import uart_console
                                uart_console.kianv()       # KianV: Linux, Zephyr, memtest, ...

What it does: selects the project, sets its clock, resets both QSPI Pmod PSRAMs (another SoC such as
TinyQV may have left them in QPI mode), holds ui_in as requested, releases reset, then bridges the
SoC UART (TX on uo_out[4], RX on ui_in[3]) to this console.

- Keys you type go to the SoC one at a time, 80 ms apart (the KianV UART has no receive FIFO;
  ttsky25a copes with 20 ms, ttgf0p2 loses characters below about 80 ms).
- Ctrl-C goes to the SoC. Press Ctrl-] to leave the console; MicroPython then soft-resets.
"""

import sys
import micropython
import uselect
import utime
from machine import UART, Pin
from ttboard.demoboard import DemoBoard

KIANV = "tt_um_kianV_rv32ima_uLinux_SoC"
KIANV_REGFILE = "tt_um_kianv_rv32_regfile"  # regfile edition (ttsky25b #970)
EXIT_KEY = b"\x1d"  # Ctrl-]


def psram_reset(tt):
    """Put both PSRAMs of the QSPI Pmod back into SPI mode (QPI exit, then reset)."""
    tt.shuttle.tt_um_chip_rom.enable()  # a project that leaves the uio pins alone
    p = tt.pins
    sd = [p.uio1.raw_pin, p.uio2.raw_pin, p.uio4.raw_pin, p.uio5.raw_pin]
    sck, flash_cs = p.uio3.raw_pin, p.uio0.raw_pin
    ram_cs = [p.uio6.raw_pin, p.uio7.raw_pin]
    flash_cs.init(Pin.OUT, value=1)
    for x in sd + [sck]:
        x.init(Pin.OUT, value=0)
    for c in ram_cs:
        c.init(Pin.OUT, value=1)

    def quad(cs, byte):
        cs(0)
        for nib in (byte >> 4, byte & 15):
            for i in range(4):
                sd[i](nib >> i & 1)
            sck(1)
            sck(0)
        cs(1)

    def spi(cs, byte):
        cs(0)
        for i in range(7, -1, -1):
            sd[0](byte >> i & 1)
            sck(1)
            sck(0)
        cs(1)

    for cs in ram_cs:
        quad(cs, 0xF5)  # exit QPI mode (harmless in SPI mode)
        spi(cs, 0x66)  # reset enable
        spi(cs, 0x99)  # reset
    utime.sleep_ms(1)
    for x in sd + [sck] + ram_cs + [flash_cs]:
        x.init(Pin.IN)


def console(project, freq, baud=115200, ui=0, char_ms=80, reset_psram=True):
    """Run `project` at `freq` Hz and bridge its UART. `ui` is driven on ui_in (bit 3 is UART RX)."""
    tt = DemoBoard.get()
    rp2350 = "RP2350" in sys.implementation._machine
    if reset_psram:
        psram_reset(tt)
    getattr(tt.shuttle, project).enable()
    tt.clock_project_PWM(freq)
    tt.reset_project(True)
    for i in (0, 1, 2, 4, 5, 6, 7):
        getattr(tt.pins, "ui_in%d" % i).raw_pin.init(Pin.OUT, value=(ui >> i) & 1)
    uart = UART(1 if rp2350 else 0, baudrate=baud, tx=tt.pins.ui_in3.raw_pin,
                rx=tt.pins.uo_out4.raw_pin, rxbuf=4096)
    utime.sleep_ms(50)
    tt.reset_project(False)
    print("[console: %s at %d Hz, %d baud. Ctrl-] to exit]" % (project, freq, baud))

    micropython.kbd_intr(-1)  # Ctrl-C goes to the SoC
    poll = uselect.poll()
    poll.register(sys.stdin, uselect.POLLIN)
    pending = bytearray()
    last = utime.ticks_ms()
    try:
        while True:
            while poll.poll(0):
                b = sys.stdin.buffer.read(1)
                if b == EXIT_KEY:
                    return
                pending += b
            now = utime.ticks_ms()
            if pending and utime.ticks_diff(now, last) >= char_ms:
                uart.write(pending[:1])
                pending = pending[1:]
                last = now
            data = uart.read()
            if data:
                sys.stdout.write(data)
    finally:
        micropython.kbd_intr(3)
        uart.deinit()
        print("\n[console closed, soft-resetting MicroPython]")
        utime.sleep_ms(100)
        import machine

        machine.soft_reset()  # return all pins and PIO to the SDK's defaults


def kianv(freq=30_000_000, baud=115200, ui=0, char_ms=80):
    """KianV uLinux SoC at 30 MHz (ttsky25a, ttgf0p2). Works for Linux, Zephyr/MicroPython and the memtest."""
    console(KIANV, freq, baud=baud, ui=ui, char_ms=char_ms)


def kianv_regfile(freq=30_000_000, baud=115200, ui=0, char_ms=80):
    """KianV regfile edition (ttsky25b #970): same images and console as kianv()."""
    console(KIANV_REGFILE, freq, baud=baud, ui=ui, char_ms=char_ms)


def kianv_rv32e(freq=25_000_000, baud=115200, char_ms=5):
    """KianV RV32E bare-metal SoC (tt07 #588). It has a receive FIFO, so input can go faster."""
    console("tt_um_kianv_bare_metal", freq, baud=baud, char_ms=char_ms)


def kianv_memtest(thorough=False, freq=30_000_000):
    """KianV memtest: ui_in[1] selects the thorough mode (sampled once at reset)."""
    console(KIANV, freq, ui=2 if thorough else 0)
