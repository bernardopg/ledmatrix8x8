# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Commands

```bash
# One-time setup
make setup

# Full local verification: pytest, mypy, isort check, PlatformIO build
make check

# Build (also runs generate_content.py automatically via PlatformIO pre-script)
make build

# Flash
make upload PORT=/dev/ttyACM0

# Serial monitor (115200 baud)
make monitor PORT=/dev/ttyACM0

# Serial diagnostic
make status PORT=/dev/ttyACM0

# Regenerate generated/project_content.h manually
make generate
```

If calling PlatformIO directly, remember it is installed in `.venv` by `make setup`: `PATH="$PWD/.venv/bin:$PATH" pio run`.

## Architecture

```
config.yaml
    └─▶ scripts/generate_content.py  (runs before every build)
            └─▶ generated/project_content.h  (auto-generated, do not edit)
                    └─▶ ledmatrix8x8_app.h   (wires matrix + effect + HA client)
                                └─▶ ledmatrix8x8.ino  (Arduino entry point)
```

**Layer breakdown:**

| Layer | Path | Role |
|---|---|---|
| App | `ledmatrix8x8_app.h` | Instantiates `LedMatrixCore`, the active effect, and HA clients; handles serial commands |
| Core | `lib/LedMatrixCore/src/LedMatrixCore.h` | Matrix abstraction: coordinate mapping, color (gamma), draw helpers |
| Effect contract | `lib/LedMatrixEffects/src/LedMatrixEffect.h` | Pure virtual: `name()`, `begin()`, `update()` |
| Effects | `lib/LedMatrixEffects/src/` | `CatAnimation.h`, `CatMessagePlayback.h` |
| Text | `lib/LedMatrixText/src/` | 5x7 pixel font + `TextMarquee` scroll logic |
| HA integration | `lib/LedMatrixIntegrations/src/HomeAssistantTextClient.h` | Polls HA REST API on a timer |
| Secrets | `include/ledmatrix8x8_secrets.h` | Gitignored; copy from `ledmatrix8x8_secrets.example.h` |

## Adding a New Effect

1. Create `lib/LedMatrixEffects/src/MyEffect.h` implementing `LedMatrixEffect` (`name`, `begin`, `update`).
2. In `ledmatrix8x8_app.h`, swap `CatMessagePlaybackEffect currentEffect(...)` for `MyEffect currentEffect(...)`.

## Config → Build Pipeline

`config.yaml` drives everything: messages, display timing, brightness, HA poll interval. The Python script normalizes text to ASCII uppercase (font charset). After editing `config.yaml`, run `make build` — the pre-script regenerates `generated/project_content.h` automatically.

## Override Priority

Serial (`TEXT:`) > Home Assistant entity > `config.yaml` messages. `CLEAR` drops serial override; HA resumes if its entity is non-empty.

## Hardware Notes

- Target: `ESP32-S3-DevKitC-1`, data pin `GPIO38`, NEO_GRB + NEO_KHZ800
- `MATRIX_ZIGZAG` and `ORIGIN_BOTTOM` flags in `ledmatrix8x8_app.h` control physical wiring layout
- No PSRAM variant by default; see `platformio.ini` comments if your module has PSRAM
