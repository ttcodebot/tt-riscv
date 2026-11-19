// First bring-up program for the KianV RV32E bare-metal SoC (tt_um_kianv_bare_metal).
// Register map: src/soc.v + src/defines_soc.vh in splinedrive/RISCV-KianV-BareMetalStyle.
// SPDX-License-Identifier: Apache-2.0
#include <stdint.h>

#define IO_BASE   0x10000000u
#define UART_DATA (*(volatile uint8_t *)(IO_BASE + 0x00))  // write: TX, read: RX (pops FIFO)
#define UART_LSR  (*(volatile uint8_t *)(IO_BASE + 0x05))  // bit0 DR, bit5 THRE, bit6 TEMT
#define PWM       (*(volatile uint32_t *)(IO_BASE + 0x14)) // 8-bit PCM -> sigma-delta on uo_out[3]
#define CYCLES    (*(volatile uint32_t *)(IO_BASE + 0x18)) // free-running clock-cycle counter
#define PSRAM     0x80000000u

#ifdef SIM
#define LED_STEP_CYCLES  20000u
#define BEEP_CYCLES      (FREQ / 500)      // 2 ms
#define MEMTEST_WORDS    64u
#else
#define LED_STEP_CYCLES  (FREQ / 4)        // 250 ms per count
#define BEEP_CYCLES      (FREQ / 10 * 3)   // 300 ms
#define MEMTEST_WORDS    16384u            // 64 KiB
#endif

static void putc_(char c) {
  while (!(UART_LSR & 0x60)) ;
  UART_DATA = c;
}
static void puts_(const char *s) { while (*s) { if (*s == '\n') putc_('\r'); putc_(*s++); } }
static void puthex(uint32_t v) {
  for (int i = 28; i >= 0; i -= 4) putc_("0123456789abcdef"[(v >> i) & 15]);
}
static void putdec(uint32_t v) {
  char b[11]; int n = 0;
  do { b[n++] = '0' + v % 10; v /= 10; } while (v);
  while (n) putc_(b[--n]);
}
static int getc_nb(void) { return (UART_LSR & 1) ? UART_DATA : -1; }


extern char stub_begin[], stub_end[];
typedef void (*stub_fn)(uint32_t until);

// Count 0..7 on uo_out[7:5] by running the wait stub from 8 PSRAM addresses (PC[18:16] = n).
static void led_count(void) {
  for (uint32_t n = 0; n < 8; n++) {
    uint32_t *dst = (uint32_t *)(PSRAM + (n << 16));
    for (uint32_t *s = (uint32_t *)stub_begin; s < (uint32_t *)stub_end; s++) *dst++ = *s;
  }
  puts_("LED count 0..7 on uo_out[7:5]: ");
  for (uint32_t n = 0; n < 8; n++) {
    putc_('0' + n);
    stub_fn f = (stub_fn)(PSRAM + (n << 16));
    f(CYCLES + LED_STEP_CYCLES);
  }
  puts_(" done\n");
}

static void beep(uint32_t hz, uint32_t cycles) {
  uint32_t half = FREQ / hz / 2, t0 = CYCLES, t = t0; int hi = 0;
  while (CYCLES - t0 < cycles) {
    PWM = hi ? 0xC0 : 0x40; hi = !hi;
    t += half; while ((int32_t)(CYCLES - t) < 0) ;
  }
  PWM = 0;
}

static int memtest(void) {
  volatile uint32_t *m = (volatile uint32_t *)(PSRAM + 0x100000);
  volatile uint8_t *b = (volatile uint8_t *)m;
  volatile uint16_t *h = (volatile uint16_t *)m;
  int err = 0;
  puts_("PSRAM test, "); putdec(MEMTEST_WORDS * 4); puts_(" bytes at 0x80100000: ");
  for (uint32_t i = 0; i < MEMTEST_WORDS; i++) m[i] = (i * 0x9E3779B1u) ^ 0xA5A55A5Au;
  for (uint32_t i = 0; i < MEMTEST_WORDS; i++) {
    uint32_t want = (i * 0x9E3779B1u) ^ 0xA5A55A5Au, got = m[i];
    if (got != want && err++ < 4) { puts_("\n  word "); puthex(i); puts_(" want "); puthex(want); puts_(" got "); puthex(got); }
  }
  // byte and halfword stores (qqspi wstrb path), then word read-back
  m[0] = 0; b[0] = 0x11; b[1] = 0x22; b[2] = 0x33; b[3] = 0x44;
  if (m[0] != 0x44332211u) { err++; puts_("\n  byte stores: got "); puthex(m[0]); }
  m[1] = 0; h[2] = 0xBEEF; h[3] = 0xCAFE;
  if (m[1] != 0xCAFEBEEFu) { err++; puts_("\n  halfword stores: got "); puthex(m[1]); }
  if (b[5] != 0xBE || h[3] != 0xCAFE) { err++; puts_("\n  sub-word loads wrong"); }
  puts_(err ? "\nPSRAM FAIL, errors: " : "PASS"); if (err) putdec(err); puts_("\n");
  return err;
}

static void help(void) {
  puts_("keys: b=beep  l=LED count  m=PSRAM test  c=cycle counter  h=help; others echo\n");
}

int main(void) {
  puts_("\nKianV RV32E bare-metal SoC: hello from silicon\n");
  puts_("build: FREQ="); putdec(FREQ); puts_(" Hz, BAUD="); putdec(BAUD); puts_("\n");
  puts_("cycle counter: "); putdec(CYCLES); puts_("\n");
  memtest();
  led_count();
  puts_("beep 1 kHz on uo_out[3] (PWM)\n");
  beep(1000, BEEP_CYCLES);
  help();
  puts_("> ");
  uint32_t last = CYCLES;
  for (;;) {
    int c = getc_nb();
    if (c < 0) {
#ifndef SIM
      if (CYCLES - last > (uint32_t)FREQ * 5) { puts_("."); last = CYCLES; } // alive tick every 5 s
#endif
      continue;
    }
    switch (c) {
      case 'b': puts_("beep\n"); beep(1000, BEEP_CYCLES); beep(1500, BEEP_CYCLES); break;
      case 'l': led_count(); break;
      case 'm': memtest(); break;
      case 'c': puts_("cycles="); putdec(CYCLES); puts_("\n"); break;
      case 'h': help(); break;
      case '\r': puts_("\n> "); break;
      default: putc_((char)(c >= 'a' && c <= 'z' ? c - 32 : c)); break;
    }
  }
}
