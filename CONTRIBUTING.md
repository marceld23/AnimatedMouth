# Contributing

Thanks for your interest! This is a small hobby project, so the process is
deliberately lightweight.

## Getting started

1. Install [PlatformIO](https://platformio.org/) (the VS Code extension is the
   easiest route).
2. Clone the repo and open the folder.
3. Connect an M5Stack Core2 and run:

   ```bash
   pio run                 # build
   pio run --target upload # flash
   pio device monitor      # serial debug output (115200 baud)
   ```

See the [README](README.md) for the architecture and tuning knobs.

## Making changes

- Keep the existing code style: 2-space indentation, `//` comments, and the
  small-class-per-file layout under `src/`.
- Tuning constants (thresholds, colors, geometry) live at the top of their
  respective files — prefer adjusting those over scattering magic numbers.
- Make sure `pio run` builds cleanly before opening a pull request.

## Pull requests

- Branch off `main`, keep PRs focused, and describe what you changed and why.
- A short note on how you tested it (which sounds / what the mouth did) is very
  welcome since the output is visual.

## Reporting bugs & ideas

Open an [issue](https://github.com/marceld23/AnimatedMouth/issues) using one of
the templates. For visual glitches, a photo or short video of the display helps
a lot.
