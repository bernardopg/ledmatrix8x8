#pragma once

/*
 * Arquivo gerado automaticamente por scripts/generate_content.py
 * Fonte: config.yaml
 */

#include <Arduino.h>

#include <LedMatrixTextTypes.h>

static const uint8_t PROJECT_BRIGHTNESS = 48;
static const uint16_t PROJECT_SCROLL_STEP_MS = 90;
static const uint16_t PROJECT_MESSAGE_PAUSE_MS = 350;
static const uint16_t PROJECT_CAT_FRAME_MS = 280;
static const uint8_t PROJECT_CAT_LOOPS = 2;
static const uint32_t PROJECT_HA_POLL_MS = 5000;
static const uint8_t PROJECT_HA_COLOR_RED = 0;
static const uint8_t PROJECT_HA_COLOR_GREEN = 255;
static const uint8_t PROJECT_HA_COLOR_BLUE = 160;

static const LedMatrixTextMessage PROJECT_MESSAGES[] = {
  {"OLA EU SOU O KLEIN SEU GATO ASSISTENTE VIRTUAL", 255, 140, 0},
  {"KANBAN: PROXIMA TAREFA", 0, 180, 255},
  {"OBSIDIAN: REVISAR NOTAS", 180, 0, 255}
};

static const size_t PROJECT_MESSAGE_COUNT =
  sizeof(PROJECT_MESSAGES) / sizeof(PROJECT_MESSAGES[0]);
