#pragma once

/*
 * LED Matrix 8x8 - painel utilitario com ESP32-S3
 *
 * Este firmware transforma a matriz 8x8 em um mini letreiro com:
 * - mensagens rolando
 * - animacao de gato em pixel art
 * - comandos simples via Serial
 *
 * O conteudo padrao vem de generated/project_content.h, gerado a partir de
 * config.yaml. Isso permite evoluir depois para sync com Obsidian/Kanban sem
 * misturar a regra do produto com o firmware principal.
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include <stdlib.h>
#include <string.h>

#include "generated/project_content.h"

// === HARDWARE ===
#define PIN_LED             38
#define MATRIX_WIDTH        8
#define MATRIX_HEIGHT       8
#define NUM_LEDS            (MATRIX_WIDTH * MATRIX_HEIGHT)
#define SERIAL_BAUD         115200
#define SERIAL_LINE_BUFFER  96

// Ajuste conforme o cabeamento real da matriz.
#define MATRIX_ZIGZAG       0
#define ORIGIN_BOTTOM       0

static Adafruit_NeoPixel matrix(NUM_LEDS, PIN_LED, NEO_GRB + NEO_KHZ800);

enum PlaybackMode {
  MODE_AUTO,
  MODE_TEXT_ONLY,
  MODE_CAT_ONLY
};

struct Glyph5x7 {
  char symbol;
  uint8_t rows[7];
};

struct CatFrame {
  uint8_t body[8];
  uint8_t accent[8];
};

static char serialLineBuffer[SERIAL_LINE_BUFFER] = {0};
static size_t serialLineLength = 0;

static char serialOverrideText[SERIAL_LINE_BUFFER] = {0};
static bool serialOverrideEnabled = false;

static uint8_t runtimeBrightness = PROJECT_BRIGHTNESS;
static PlaybackMode playbackMode = MODE_AUTO;

static const Glyph5x7 FONT_5X7[] = {
  {' ', {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},
  {'!', {0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04}},
  {'-', {0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00}},
  {'.', {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C}},
  {'/', {0x01, 0x02, 0x04, 0x08, 0x10, 0x00, 0x00}},
  {'0', {0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}},
  {'1', {0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}},
  {'2', {0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}},
  {'3', {0x1E, 0x01, 0x01, 0x06, 0x01, 0x01, 0x1E}},
  {'4', {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}},
  {'5', {0x1F, 0x10, 0x10, 0x1E, 0x01, 0x01, 0x1E}},
  {'6', {0x0E, 0x10, 0x10, 0x1E, 0x11, 0x11, 0x0E}},
  {'7', {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}},
  {'8', {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}},
  {'9', {0x0E, 0x11, 0x11, 0x0F, 0x01, 0x01, 0x0E}},
  {':', {0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00}},
  {'?', {0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04}},
  {'A', {0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}},
  {'B', {0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}},
  {'C', {0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}},
  {'D', {0x1C, 0x12, 0x11, 0x11, 0x11, 0x12, 0x1C}},
  {'E', {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}},
  {'F', {0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}},
  {'G', {0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0F}},
  {'H', {0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}},
  {'I', {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x1F}},
  {'J', {0x01, 0x01, 0x01, 0x01, 0x11, 0x11, 0x0E}},
  {'K', {0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}},
  {'L', {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}},
  {'M', {0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}},
  {'N', {0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11}},
  {'O', {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
  {'P', {0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}},
  {'Q', {0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}},
  {'R', {0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}},
  {'S', {0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}},
  {'T', {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}},
  {'U', {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}},
  {'V', {0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}},
  {'W', {0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A}},
  {'X', {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}},
  {'Y', {0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04}},
  {'Z', {0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}}
};

static const uint8_t UNKNOWN_GLYPH[7] = {
  0x0E, 0x11, 0x01, 0x02, 0x04, 0x00, 0x04
};

static const CatFrame CAT_FRAMES[] = {
  {
    {0x81, 0xC3, 0xFF, 0xFF, 0xFF, 0x7E, 0x5A, 0xA5},
    {0x00, 0x24, 0x00, 0x42, 0x18, 0x24, 0x00, 0x00}
  },
  {
    {0x81, 0xC3, 0xFF, 0xFF, 0xFF, 0x7E, 0x5A, 0xA5},
    {0x00, 0x24, 0x00, 0x3C, 0x18, 0x24, 0x00, 0x00}
  },
  {
    {0x81, 0xC3, 0xFF, 0xFF, 0xFF, 0x7E, 0x52, 0x49},
    {0x00, 0x24, 0x00, 0x42, 0x18, 0x24, 0x20, 0x10}
  },
  {
    {0x81, 0xC3, 0xFF, 0xFF, 0xFF, 0x7E, 0x52, 0x92},
    {0x00, 0x24, 0x00, 0x42, 0x18, 0x24, 0x04, 0x08}
  }
};

inline uint16_t xyToIndex(uint8_t x, uint8_t y) {
#if ORIGIN_BOTTOM
  y = (MATRIX_HEIGHT - 1) - y;
#endif

#if MATRIX_ZIGZAG
  if (y & 0x01) {
    x = (MATRIX_WIDTH - 1) - x;
  }
#endif

  return (uint16_t)y * MATRIX_WIDTH + x;
}

inline void setXY(uint8_t x, uint8_t y, uint32_t color) {
  if (x >= MATRIX_WIDTH || y >= MATRIX_HEIGHT) {
    return;
  }

  matrix.setPixelColor(xyToIndex(x, y), color);
}

inline void clearMatrix() {
  matrix.clear();
}

inline void showMatrix() {
  matrix.show();
}

inline uint32_t gammaColor(uint8_t red, uint8_t green, uint8_t blue) {
  return matrix.gamma32(matrix.Color(red, green, blue));
}

inline char uppercaseAscii(char value) {
  if (value >= 'a' && value <= 'z') {
    return (char)(value - ('a' - 'A'));
  }

  return value;
}

inline const uint8_t *glyphRowsFor(char symbol) {
  const char normalized = uppercaseAscii(symbol);

  for (size_t i = 0; i < sizeof(FONT_5X7) / sizeof(FONT_5X7[0]); i++) {
    if (FONT_5X7[i].symbol == normalized) {
      return FONT_5X7[i].rows;
    }
  }

  return UNKNOWN_GLYPH;
}

inline void drawGlyph(char symbol, int16_t originX, uint8_t originY, uint32_t color) {
  const uint8_t *rows = glyphRowsFor(symbol);

  for (uint8_t y = 0; y < 7; y++) {
    const uint8_t rowBits = rows[y];
    for (uint8_t x = 0; x < 5; x++) {
      if (rowBits & (1 << (4 - x))) {
        const int16_t drawX = originX + x;
        const int16_t drawY = originY + y;
        if (drawX >= 0 && drawY >= 0) {
          setXY((uint8_t)drawX, (uint8_t)drawY, color);
        }
      }
    }
  }
}

inline int16_t textWidthPixels(const char *text) {
  const int16_t glyphWidth = 6;
  const int16_t length = (int16_t)strlen(text);
  if (length <= 0) {
    return 0;
  }

  return (length * glyphWidth) - 1;
}

inline void drawTextFrame(const char *text, int16_t offsetX, uint32_t color) {
  clearMatrix();

  int16_t cursorX = offsetX;
  for (size_t i = 0; text[i] != '\0'; i++) {
    drawGlyph(text[i], cursorX, 0, color);
    cursorX += 6;
  }

  showMatrix();
}

inline void drawMaskRow(uint8_t y, uint8_t rowBits, uint32_t color) {
  for (uint8_t x = 0; x < 8; x++) {
    if (rowBits & (0x80 >> x)) {
      setXY(x, y, color);
    }
  }
}

inline void drawCatFrame(const CatFrame &frame) {
  const uint32_t bodyColor = gammaColor(255, 160, 40);
  const uint32_t accentColor = gammaColor(32, 220, 255);

  clearMatrix();
  for (uint8_t y = 0; y < 8; y++) {
    drawMaskRow(y, frame.body[y], bodyColor);
    drawMaskRow(y, frame.accent[y], accentColor);
  }
  showMatrix();
}

inline void applyBrightness(uint8_t brightness) {
  runtimeBrightness = brightness;
  matrix.setBrightness(runtimeBrightness);
  showMatrix();
}

inline void printStatus() {
  Serial.println();
  Serial.println("STATUS");
  Serial.print("  brightness: ");
  Serial.println(runtimeBrightness);
  Serial.print("  mode: ");
  if (playbackMode == MODE_AUTO) {
    Serial.println("AUTO");
  } else if (playbackMode == MODE_TEXT_ONLY) {
    Serial.println("TEXT");
  } else {
    Serial.println("CAT");
  }
  Serial.print("  serial override: ");
  Serial.println(serialOverrideEnabled ? serialOverrideText : "(disabled)");
  Serial.println();
}

inline void printCommandHelp() {
  Serial.println("Comandos:");
  Serial.println("  TEXT:<mensagem>   envia mensagem temporaria via serial");
  Serial.println("  CLEAR             remove a mensagem temporaria");
  Serial.println("  MODE:AUTO         alterna mensagens e gato");
  Serial.println("  MODE:TEXT         mostra apenas mensagens");
  Serial.println("  MODE:CAT          mostra apenas o gato");
  Serial.println("  BRIGHTNESS:<0-255>");
  Serial.println("  STATUS");
  Serial.println("  HELP");
  Serial.println();
}

inline void copySanitizedAscii(const char *input, char *output, size_t outputSize) {
  if (outputSize == 0) {
    return;
  }

  size_t outIndex = 0;
  for (size_t i = 0; input[i] != '\0' && outIndex + 1 < outputSize; i++) {
    const char current = uppercaseAscii(input[i]);
    const bool isAllowed =
      (current >= 'A' && current <= 'Z') ||
      (current >= '0' && current <= '9') ||
      current == ' ' || current == ':' || current == '-' ||
      current == '.' || current == '/' || current == '!' || current == '?';

    output[outIndex++] = isAllowed ? current : ' ';
  }

  output[outIndex] = '\0';
}

inline void handleCommand(const char *line) {
  if (strncmp(line, "TEXT:", 5) == 0) {
    copySanitizedAscii(line + 5, serialOverrideText, sizeof(serialOverrideText));
    serialOverrideEnabled = serialOverrideText[0] != '\0';
    Serial.print("Mensagem serial: ");
    Serial.println(serialOverrideEnabled ? serialOverrideText : "(vazia)");
    return;
  }

  if (strcmp(line, "CLEAR") == 0) {
    serialOverrideText[0] = '\0';
    serialOverrideEnabled = false;
    Serial.println("Mensagem serial removida.");
    return;
  }

  if (strcmp(line, "MODE:AUTO") == 0) {
    playbackMode = MODE_AUTO;
    Serial.println("Modo alterado para AUTO.");
    return;
  }

  if (strcmp(line, "MODE:TEXT") == 0) {
    playbackMode = MODE_TEXT_ONLY;
    Serial.println("Modo alterado para TEXT.");
    return;
  }

  if (strcmp(line, "MODE:CAT") == 0) {
    playbackMode = MODE_CAT_ONLY;
    Serial.println("Modo alterado para CAT.");
    return;
  }

  if (strncmp(line, "BRIGHTNESS:", 11) == 0) {
    const long parsed = strtol(line + 11, NULL, 10);
    if (parsed < 0 || parsed > 255) {
      Serial.println("Brightness invalido. Use 0-255.");
      return;
    }

    applyBrightness((uint8_t)parsed);
    Serial.print("Brightness ajustado para ");
    Serial.println(runtimeBrightness);
    return;
  }

  if (strcmp(line, "STATUS") == 0) {
    printStatus();
    return;
  }

  if (strcmp(line, "HELP") == 0) {
    printCommandHelp();
    return;
  }

  Serial.print("Comando desconhecido: ");
  Serial.println(line);
}

inline void processSerial() {
  while (Serial.available() > 0) {
    const char incoming = (char)Serial.read();

    if (incoming == '\r') {
      continue;
    }

    if (incoming == '\n') {
      serialLineBuffer[serialLineLength] = '\0';
      if (serialLineLength > 0) {
        handleCommand(serialLineBuffer);
      }
      serialLineLength = 0;
      serialLineBuffer[0] = '\0';
      continue;
    }

    if (serialLineLength + 1 < sizeof(serialLineBuffer)) {
      serialLineBuffer[serialLineLength++] = incoming;
      serialLineBuffer[serialLineLength] = '\0';
    } else {
      serialLineLength = 0;
      serialLineBuffer[0] = '\0';
      Serial.println("Linha serial longa demais; buffer descartado.");
    }
  }
}

inline void waitWithSerial(uint16_t totalMs) {
  const uint32_t start = millis();
  while ((millis() - start) < totalMs) {
    processSerial();
    delay(5);
  }
}

inline void scrollMessage(const char *text, uint32_t color) {
  const int16_t width = textWidthPixels(text);

  for (int16_t offset = MATRIX_WIDTH; offset > -width; offset--) {
    processSerial();
    drawTextFrame(text, offset, color);
    waitWithSerial(PROJECT_SCROLL_STEP_MS);
  }

  waitWithSerial(PROJECT_MESSAGE_PAUSE_MS);
}

inline void playCatAnimation(uint8_t loops) {
  for (uint8_t loopIndex = 0; loopIndex < loops; loopIndex++) {
    for (size_t frameIndex = 0; frameIndex < sizeof(CAT_FRAMES) / sizeof(CAT_FRAMES[0]); frameIndex++) {
      processSerial();
      drawCatFrame(CAT_FRAMES[frameIndex]);
      waitWithSerial(PROJECT_CAT_FRAME_MS);

      if (playbackMode == MODE_TEXT_ONLY) {
        return;
      }
    }
  }
}

inline void playConfiguredMessages() {
  for (size_t i = 0; i < PROJECT_MESSAGE_COUNT; i++) {
    processSerial();

    if (playbackMode == MODE_CAT_ONLY) {
      return;
    }

    const ProjectMessage &message = PROJECT_MESSAGES[i];
    scrollMessage(message.text, gammaColor(message.red, message.green, message.blue));
  }
}

inline void playSerialOverride() {
  scrollMessage(serialOverrideText, gammaColor(255, 255, 255));
}

inline void ledmatrix8x8Setup() {
  Serial.begin(SERIAL_BAUD);
  delay(1200);

  matrix.begin();
  matrix.setBrightness(PROJECT_BRIGHTNESS);
  matrix.clear();
  matrix.show();

  applyBrightness(PROJECT_BRIGHTNESS);

  Serial.println();
  Serial.println("==== LEDMATRIX8X8 ====");
  Serial.print("GPIO: ");
  Serial.println(PIN_LED);
  Serial.print("Mensagens padrao: ");
  Serial.println(PROJECT_MESSAGE_COUNT);
  Serial.println("Modo inicial: AUTO");
  Serial.println("Conteudo util gerado de config.yaml");
  printCommandHelp();
}

inline void ledmatrix8x8Loop() {
  processSerial();

  if (serialOverrideEnabled && playbackMode != MODE_CAT_ONLY) {
    playSerialOverride();
    if (playbackMode == MODE_AUTO) {
      playCatAnimation(PROJECT_CAT_LOOPS);
    }
    return;
  }

  if (playbackMode != MODE_CAT_ONLY) {
    playConfiguredMessages();
  }

  if (playbackMode != MODE_TEXT_ONLY) {
    playCatAnimation(PROJECT_CAT_LOOPS);
  }
}
