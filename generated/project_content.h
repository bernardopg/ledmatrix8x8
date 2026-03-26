#pragma once

/*
 * Arquivo gerado automaticamente por scripts/generate_content.py
 * Fonte: config.yaml
 */

#include <Arduino.h>

struct ProjectMessage {
  const char *text;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

static const uint8_t PROJECT_BRIGHTNESS = 48;
static const uint16_t PROJECT_SCROLL_STEP_MS = 90;
static const uint16_t PROJECT_MESSAGE_PAUSE_MS = 350;
static const uint16_t PROJECT_CAT_FRAME_MS = 140;
static const uint8_t PROJECT_CAT_LOOPS = 2;

static const ProjectMessage PROJECT_MESSAGES[] = {
  {"FOCO NA ENTREGA", 255, 140, 0},
  {"KANBAN: PROXIMA TAREFA", 0, 180, 255},
  {"OBSIDIAN: REVISAR NOTAS", 180, 0, 255}
};

static const size_t PROJECT_MESSAGE_COUNT =
  sizeof(PROJECT_MESSAGES) / sizeof(PROJECT_MESSAGES[0]);
