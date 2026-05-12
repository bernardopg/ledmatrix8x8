#!/usr/bin/env python3
"""Gera generated/project_content.h a partir de config.yaml.

O arquivo config.yaml e escrito em YAML compativel com JSON para que o projeto
continue usando apenas a biblioteca padrao do Python.
"""

from __future__ import annotations

import json
import pathlib
import unicodedata
from typing import Any


def detect_root() -> pathlib.Path:
    if "__file__" in globals():
        return pathlib.Path(__file__).resolve().parent.parent

    platformio_import = globals().get("Import")
    if callable(platformio_import):
        platformio_import("env")
        platformio_env: Any = globals().get("env", {})
        project_dir = platformio_env.get("PROJECT_DIR")
        if project_dir:
            return pathlib.Path(project_dir).resolve()

    return pathlib.Path.cwd().resolve()


ROOT = detect_root()
CONFIG_PATH = ROOT / "config.yaml"
OUTPUT_PATH = ROOT / "generated" / "project_content.h"

ALLOWED_PUNCTUATION = set(" :-./!?")


def clamp(value: int, minimum: int, maximum: int) -> int:
    return max(minimum, min(maximum, value))


def normalize_text(raw: str) -> str:
    normalized = (
        unicodedata.normalize("NFKD", raw)
        .encode("ascii", "ignore")
        .decode("ascii")
        .upper()
    )

    cleaned = []
    previous_space = False
    for char in normalized:
        allowed = char.isalnum() or char in ALLOWED_PUNCTUATION
        current = char if allowed else " "

        if current == " ":
            if previous_space:
                continue
            previous_space = True
        else:
            previous_space = False

        cleaned.append(current)

    final_text = "".join(cleaned).strip()
    return final_text or "OK"


def load_config() -> dict:
    return json.loads(CONFIG_PATH.read_text(encoding="utf-8"))


def render_header(config: dict) -> str:
    display = config.get("display", {})
    messages = config.get("messages", [])
    home_assistant = config.get("home_assistant", {})

    if not messages:
        messages = [{"text": "CONFIGURE UMA MENSAGEM", "color": [255, 255, 255]}]

    brightness = clamp(int(display.get("brightness", 48)), 1, 255)
    scroll_step_ms = clamp(int(display.get("scroll_step_ms", 90)), 10, 1000)
    message_pause_ms = clamp(int(display.get("message_pause_ms", 350)), 0, 4000)
    cat_frame_ms = clamp(int(display.get("cat_frame_ms", 140)), 40, 2000)
    cat_loops = clamp(int(display.get("cat_loops", 2)), 1, 20)
    ha_poll_ms = clamp(int(home_assistant.get("poll_ms", 5000)), 1000, 60000)
    ha_color = home_assistant.get("message_color", [0, 255, 160])
    if not isinstance(ha_color, list) or len(ha_color) != 3:
        ha_color = [0, 255, 160]

    ha_red = clamp(int(ha_color[0]), 0, 255)
    ha_green = clamp(int(ha_color[1]), 0, 255)
    ha_blue = clamp(int(ha_color[2]), 0, 255)

    rendered_messages = []
    for item in messages:
        text = normalize_text(str(item.get("text", "")))
        color = item.get("color", [255, 255, 255])
        if not isinstance(color, list) or len(color) != 3:
            color = [255, 255, 255]

        red = clamp(int(color[0]), 0, 255)
        green = clamp(int(color[1]), 0, 255)
        blue = clamp(int(color[2]), 0, 255)

        escaped_text = text.replace("\\", "\\\\").replace('"', '\\"')
        rendered_messages.append(
            f'  {{"{escaped_text}", {red}, {green}, {blue}}}'
        )

    messages_block = ",\n".join(rendered_messages)

    return f"""#pragma once

/*
 * Arquivo gerado automaticamente por scripts/generate_content.py
 * Fonte: config.yaml
 */

#include <Arduino.h>

#include <LedMatrixTextTypes.h>

static const uint8_t PROJECT_BRIGHTNESS = {brightness};
static const uint16_t PROJECT_SCROLL_STEP_MS = {scroll_step_ms};
static const uint16_t PROJECT_MESSAGE_PAUSE_MS = {message_pause_ms};
static const uint16_t PROJECT_CAT_FRAME_MS = {cat_frame_ms};
static const uint8_t PROJECT_CAT_LOOPS = {cat_loops};
static const uint32_t PROJECT_HA_POLL_MS = {ha_poll_ms};
static const uint8_t PROJECT_HA_COLOR_RED = {ha_red};
static const uint8_t PROJECT_HA_COLOR_GREEN = {ha_green};
static const uint8_t PROJECT_HA_COLOR_BLUE = {ha_blue};

static const LedMatrixTextMessage PROJECT_MESSAGES[] = {{
{messages_block}
}};

static const size_t PROJECT_MESSAGE_COUNT =
  sizeof(PROJECT_MESSAGES) / sizeof(PROJECT_MESSAGES[0]);
"""


def main() -> None:
    config = load_config()
    OUTPUT_PATH.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT_PATH.write_text(render_header(config), encoding="utf-8")
    print(f"generated {OUTPUT_PATH.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
