# Breakout on TinyQV (C)

A Breakout game in C for TinyQV "Asteroids", with VGA video, gamepad input and sound. It uses Mike Bell's
[tinyQV-sdk](https://github.com/MichaelBell/tinyQV-sdk) and tinyQV toolchain. Source:
[`examples/tinyqv-breakout`](../examples/tinyqv-breakout).

Tested on ttsky25a `tt_um_tt_tinyQV` (#495), with a Gamepad Pmod and controller.

## What you need

| Header                | Pmod                                                                                                                         |
| --------------------- | ---------------------------------------------------------------------------------------------------------------------------- |
| Output (`uo_out`)     | [TinyVGA Pmod](https://store.tinytapeout.com/products/TinyVGA-Pmod-p678647356), to a monitor                                 |
| Input (`ui_in`)       | [Gamepad Pmod](https://store.tinytapeout.com/products/Gamepad-Pmod-board-p741891425), with one or two SNES-style controllers |
| Bidirectional (`uio`) | QSPI Pmod (the program runs from its flash)                                                                                  |

The monitor must accept 1024x768 at 60 Hz. The game draws a 256x192 picture, scaled up 4x.

## Just run it

1. Download `tinyqv-breakout.bin` from the [latest release](../../../releases/latest).
2. In the [TinyQV programmer](https://program.tinyqv.com) choose **Custom**, pick the file, design **495**,
   latency **2**, **64** MHz, then **FLASH AND RUN**.

## Playing

| Control      | Action                                |
| ------------ | ------------------------------------- |
| Left / right | Move the paddle                       |
| Hold B       | Move the paddle faster                |
| A or Start   | Serve; start a new game after the end |

Either controller port works. Without a controller at power-up, the game plays itself (demo mode,
`DEMO` in the top bar). Plug a controller in and reset to play.

- 6 rows of 14 bricks. The bottom rows score 1 point, the middle rows 4, the top rows 7.
- The ball speeds up after 4 and 12 hits, and when it reaches the top two rows. Each new level
  starts a little faster.
- 3 lives. Where the ball hits the paddle sets its angle.

## Sound

The sound comes from the PiecewiseOrionSynth peripheral (33) as a PWM signal on `uo_out[6]`. That pin is
blue bit 0 of the TinyVGA Pmod; the game's colours never set that bit. The PWM shows on the monitor as
faint blue vertical streaks that move with the sound, which the game keeps for an old-CRT look.

To hear it, set **DIP switch 3** (`ui_in[2]`) on before reset. The sound then also goes to `uio[7]`, for an
Audio Pmod on the bidirectional header with the QSPI Pmod stacked on top. Leave the switch off without an
Audio Pmod: `uio[7]` is the chip select of the QSPI Pmod's PSRAM B. With the Audio Pmod fitted, PSRAM B is
held disabled; the game does not use it.

## Build it

The toolchain is a Linux x86-64 build that needs glibc 2.38 or newer (Ubuntu 24.04 or later, Debian 13).

```sh
scripts/build-tinyqv-breakout.sh      # -> out/tinyqv-breakout/tinyqv-breakout.bin
```

The script downloads the toolchain (about 1 GB) and the SDK at the commits pinned in
[`pins.env`](../pins.env), builds the SDK's libraries, then the game. To use a toolchain you already
have, set `RISCV_TOOLCHAIN`, e.g. `RISCV_TOOLCHAIN=/opt/tinyQV`.

Use toolchain 16.1.0-tqv-2.1 or newer. Older builds assemble some `mul16` instructions (TinyQV's
16-bit multiply) with their operands swapped, which breaks the SDK's graphics routines.

Build options, passed with `make EXTRA_CFLAGS=...` in `examples/tinyqv-breakout`:

| Option         | Effect                                                                  |
| -------------- | ----------------------------------------------------------------------- |
| `-DAUDIO_UIO7` | Always route the sound to `uio[7]` (Audio Pmod), without the DIP switch |
| `-DSHOW_LOAD`  | Show the CPU load in the top bar: VGA lines spent per frame (of 806)    |
| `-DTEXTURE=n`  | Brick and paddle textures, 1-6 (see `breakout.c`). Default 0: solid     |
| `-DSEED=n`     | Random seed of the demo player                                          |

## How it works

- **Video.** The vga_gfx peripheral (9) shows a 256x192, 2-bit-per-pixel frame buffer in PSRAM. The CPU is
  slow (one instruction per 8 clocks, code read from flash), so the game never redraws the whole screen.
  At the start of vertical blank it draws only what changed (ball, paddle, broken bricks, score), then
  runs the game logic for the next frame.
- **Input.** `gamepad.h` from the SDK; the game ORs the buttons of both controllers.
- **Sound.** Four synth voices for the paddle, walls, bricks (pitch by row) and a lost ball, plus short
  tunes for a new game, a cleared level and game over.
- **Maths.** Integers only, 8.8 fixed point for the ball. Multiplies use TinyQV's `mul16` instruction
  (`mul32x16()` from the SDK's `mul.h`); there is no division.

The image is about 7 KB at flash offset 0, so a KianV Linux image on the same Pmod is not touched.
