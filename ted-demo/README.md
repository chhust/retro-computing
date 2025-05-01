# TED Sound Demo

## for the Commodore 264 Series (C16, C116, Plus/4)

---

This program in Commodore BASIC V3.5 provides a sound demo for the **MOS 7360/8360 TED (Text Editing Device)** chip used in the Commodore 264 computer series introduced in 1984. The program allows users to experiment interactively with TED's very limited sound capabilities: adjust pitch, toggle channels, switch between noise and tone modes, and change the volume.

**This code was written in 2024 as part of the DFG-funded research project "Cultures of Home Computer Music" at the University for Music and Theatre, Leipzig, Germany.**

---

## Features

- Control of both tone generators (channels 1 & 2)
- Manual and interactive pitch adjustments
- Volume control
- Switch channel 2 between noise and square (rectangle) wave
- Real-time toggling of channels
- Demonstrates TED-specific characteristics, such as the unevenly ordered frequency steps, noise synthesis, and beating and interference effects achievable by the two channels being set to similar, but not identical values.

---

## Controls

| Key(s)               | Function                                        |
| -------------------- | ----------------------------------------------- |
| `Q`, `A`             | Adjust pitch of channel 1 (± 1)                 |
| `Shift+Q`, `Shift+A` | Adjust pitch of channel 1 (± 10)                |
| `O`, `K`             | Adjust pitch of channel 2 (± 1)                 |
| `Shift+O`, `Shift+K` | Adjust pitch of channel 2 (± 10)                |
| `I`                  | Manual pitch input for both channels            |
| `V`                  | Toggle channel 1 on / off                       |
| `B`                  | Toggle channel 2 on / off                       |
| `Space`              | Toggle sound mode of channel 2 (Square / Noise) |
| `0`–`8`              | Set overall volume level                        |
| `ESC`                | Exit the program                                |

---

## How It Works

The program directly manipulates TED sound registers via `POKE` and reads input with `GET`. The `PEEK` command is used to preserve or combine register bits. It uses the following memory-mapped registers:

- `$FF4E` (65294): Channel 1 pitch (low byte)
- `$FF4F` (65295): Channel 2 pitch (low byte)
- `$FF50` (65296): Channel 2 pitch (high byte)
- `$FF52` (65298): Channel 1 pitch (high bits & control)
- `$FF51` (65297): Channel on / off, mode, volume

As usual, values are encoded in little-endian style. Low bytes can be written directly to memory. Channel 1's high bytes and the handling of the multifunctional settings register need bit-wise operations, achieved by bit-masking with combinations of `POKE` and `PEEK` commands with logical operators `AND`, `OR`, and `NOT`.

TED produces simple square waves and noise, but its limitations lead to some strange quirks in tone shaping and interference.

---

## Requirements

A Commodore 264 computer or an emulator such as `VICE` or `YAPE`. It was tested via the `C64 Forever` environment. The easiest way to get this running is by loading the code in `CBM prg Studio`. Within the emulator, load the code, and start with `RUN`.

---

christoph.hust@hmt-leipzig.de