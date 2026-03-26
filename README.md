<p align="center">
  <img src="https://capsule-render.vercel.app/api?type=waving&color=0:0F172A,45:0F766E,100:F59E0B&height=240&section=header&text=ledmatrix8x8&fontSize=48&fontColor=FFFFFF&fontAlignY=38&desc=ESP32-S3%20pixel%20badge%20with%20importable%20effects&descAlignY=58" alt="ledmatrix8x8 banner" />
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Board-ESP32--S3-0F766E?style=for-the-badge&logo=espressif&logoColor=white" alt="ESP32-S3" />
  <img src="https://img.shields.io/badge/Matrix-WS2812B%208x8-F59E0B?style=for-the-badge&logo=arduino&logoColor=white" alt="WS2812B 8x8" />
  <img src="https://img.shields.io/badge/Build-PlatformIO-1F2937?style=for-the-badge&logo=platformio&logoColor=white" alt="PlatformIO" />
  <img src="https://img.shields.io/badge/Firmware-Cat%20Face-1D4ED8?style=for-the-badge&logo=githubsponsors&logoColor=white" alt="Cat Face" />
  <img src="https://img.shields.io/badge/Architecture-Importable%20Effects-7C3AED?style=for-the-badge&logo=gitbook&logoColor=white" alt="Importable effects" />
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Current%20state-Flashed%20and%20running-16A34A?style=flat-square" alt="Flashed and running" />
  <img src="https://img.shields.io/badge/Visual%20direction-Hardware%20badge%20with%20personality-0F766E?style=flat-square" alt="Hardware badge with personality" />
  <img src="https://img.shields.io/badge/Code%20style-Core%20%2B%20Effects%20%2B%20App-334155?style=flat-square" alt="Core + Effects + App" />
</p>

<p align="center">
  <a href="#pt-br">PT-BR</a> •
  <a href="#english">English</a> •
  <a href="#visual-preview">Visual Preview</a> •
  <a href="#architecture--schema">Architecture</a> •
  <a href="#project-chart">Chart</a> •
  <a href="#library-layout">Library Layout</a> •
  <a href="#quick-start">Quick Start</a>
</p>

## PT-BR

`ledmatrix8x8` é um badge/painel NeoPixel 8x8 baseado em `ESP32-S3`, pensado para virar uma base de efeitos visuais importáveis. O firmware carregado no dispositivo hoje mostra um rosto de gato estático com identidade própria, mas a arquitetura foi organizada para permitir trocar o comportamento com baixo atrito: um núcleo da matriz, uma interface de efeitos e efeitos separados como biblioteca local.

O objetivo deixou de ser “um sketch de teste” e passou a ser um artefato de hardware pequeno, visualmente marcante e com código reaproveitável.

## English

`ledmatrix8x8` is an `ESP32-S3` NeoPixel 8x8 badge built around reusable visual effects. The device currently runs a cat-face effect, while the codebase is organized as a small importable firmware stack: matrix core, effect interface, and swappable local effects.

The goal is no longer a throwaway test sketch. It is now a compact hardware artifact with stronger visual direction and a reusable firmware structure.

## Visual Preview

<p align="center">
  <img src="./docs/assets/cat-preview.svg" alt="Cat effect visual preview" width="900" />
</p>

## Current Snapshot

| Item | Status |
| --- | --- |
| Current deployed effect | Static cat face |
| Device target | `ESP32-S3-DevKitC-1` |
| LED matrix | `WS2812B 8x8` |
| Data pin | `GPIO38` |
| Build system | `PlatformIO` |
| Firmware style | Local library-based effects |

## Architecture / Schema

```mermaid
flowchart LR
  A["config.yaml"] --> B["scripts/generate_content.py"]
  B --> C["generated/project_content.h"]
  C --> D["ledmatrix8x8_app.h"]
  E["LedMatrixCore"] --> D
  F["LedMatrixEffect"] --> G["CatAnimationEffect"]
  G --> D
  D --> H["ESP32-S3 + WS2812B 8x8"]
```

## Project Chart

```mermaid
pie showData title Current project emphasis
  "Reusable firmware structure" : 35
  "Visual effect design" : 30
  "Hardware integration" : 20
  "Future automation hooks" : 15
```

## Library Layout

| Layer | Path | Responsibility |
| --- | --- | --- |
| App | `ledmatrix8x8_app.h` | Wires the selected effect into the runtime |
| Core | `lib/LedMatrixCore/src/LedMatrixCore.h` | Matrix abstraction, coordinates, color, draw helpers |
| Effect contract | `lib/LedMatrixEffects/src/LedMatrixEffect.h` | Common interface for importable effects |
| Effect | `lib/LedMatrixEffects/src/CatAnimation.h` | Current cat-face effect |
| Generated config | `generated/project_content.h` | Build-time constants generated from config |
| Content source | `config.yaml` | Human-edited project config |

## Why This Direction

- The firmware is now structured to swap effects without rewriting the whole project.
- The matrix core can be reused by text effects, status indicators, or future animations.
- The current cat effect gives the badge a visual identity instead of looking like a generic LED test.
- The repo is ready to grow into multiple behaviors rather than a single monolithic sketch.

## Quick Start

1. Edit `config.yaml`
2. Run `python scripts/generate_content.py`
3. Run `pio run`
4. Flash with `pio run -t upload --upload-port /dev/ttyACM0`

## How To Swap The Active Effect

The main app only instantiates one effect:

```cpp
#include <CatAnimation.h>

static CatAnimationEffect currentEffect(PROJECT_CAT_FRAME_MS);
```

To add another behavior, create a new header under `lib/LedMatrixEffects/src/` implementing `LedMatrixEffect`, then swap the instance in `ledmatrix8x8_app.h`.

## Hardware

- `ESP32-S3-DevKitC-1`
- `WS2812B 8x8`
- `GPIO38`

If your matrix is wired differently, update `MATRIX_ZIGZAG` and `ORIGIN_BOTTOM` in `ledmatrix8x8_app.h`.

## Useful Directions

- Build more effects: blink, idle pulse, side-profile cat, tiny text scroller, status glyphs.
- Add an effect selector later without changing the core matrix library.
- Reuse the same runtime for Kanban/Obsidian-inspired modes in future iterations.

## Reference Docs

- Product context: [PROJECT_CONTEXT.md](PROJECT_CONTEXT.md)
- Inspirations and integrations: [docs/INSPIRATIONS.md](docs/INSPIRATIONS.md)
