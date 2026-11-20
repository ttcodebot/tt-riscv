/* Hardware access for Breakout on TinyQV "Asteroids" (ttsky25a #495). */
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <peripherals/vga_gfx.h>
#include <gamepad.h>
#include <mul.h>

/* Current VGA line 0..805 (0..767 visible, 768.. vertical blank). */
static inline int hal_scanline(void) {
    uint32_t hi, lo, hi2;
    do {
        asm volatile("lbu %0, 0x243(tp)" : "=r"(hi));
        asm volatile("lbu %0, 0x242(tp)" : "=r"(lo));
        asm volatile("lbu %0, 0x243(tp)" : "=r"(hi2));
    } while (hi != hi2);
    return mul32x16(hi, 48) + (lo & 63);
}

static inline bool hal_in_vblank(void) {
    uint32_t hi;
    asm volatile("lbu %0, 0x243(tp)" : "=r"(hi));
    return hi == 16;
}

/* Wait for the start of the next vertical blank. */
static inline void hal_wait_vblank(void) {
    while (hal_in_vblank()) {}
    while (!hal_in_vblank()) {}
}

/* PiecewiseOrionSynth, user peripheral 33. 16-bit registers at byte offsets. */
#define PWL_BASE 0x8000640
static inline void hal_pwl(int ch, int reg, int v) {
    *(volatile uint16_t *)(PWL_BASE + (ch << 4) + reg) = v;
}

/* Route the synth to an output pin.
 * Default: uo_out[6] (TinyVGA B0) carries the synth's left PWM output; the game palette
 * never sets B0, so the picture is unchanged and the audio is visible to anything that
 * samples uo_out (the RP2350 capture does).
 * -DAUDIO_UIO7: route to uio[7] via AUDIO_FUNC_SEL instead (an audio Pmod there). On the
 * QSPI Pmod uio[7] is PSRAM B's chip select, so only use this without PSRAM B fitted. */
static inline void hal_audio_route(void) {
#ifdef AUDIO_UIO7
    *(volatile uint32_t *)0x8000050 = 8 + 6;      /* audio_select | PWL left */
    /* Also on uo_out[6] (TinyVGA B0): on a monitor the synth's ~1 MHz PWM shows as faint blue
     * vertical streaks that shift with the sound, an old-CRT look the game keeps on purpose. */
    *(volatile uint32_t *)(0x8000060 + 4 * 6) = 33;
#else
    /* Runtime choice: ui_in[2] high at start (DIP switch 3 on) also routes the synth to uio[7],
     * for an Audio Pmod on the BIDIR header with the QSPI Pmod stacked on it (the Audio Pmod
     * keeps the stacked IO7, PSRAM B's select, high). Leave it off without an Audio Pmod. */
    if (*(volatile uint32_t *)0x8000044 & (1u << 2))
        *(volatile uint32_t *)0x8000050 = 8 + 6;  /* audio_select | PWL left on uio[7] (same channel as uo_out[6]) */
    *(volatile uint32_t *)(0x8000060 + 4 * 6) = 33; /* uo_out[6] func_sel = peripheral 33 */
#endif
}

