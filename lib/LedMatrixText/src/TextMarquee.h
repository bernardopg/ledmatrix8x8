#pragma once

#include <Arduino.h>

#include <LedMatrixCore.h>

#include "PixelFont5x7.h"

class TextMarquee {
 public:
  TextMarquee(uint16_t stepIntervalMs, uint16_t pauseAfterScrollMs)
      : stepIntervalMs_(stepIntervalMs),
        pauseAfterScrollMs_(pauseAfterScrollMs) {}

  void setMessage(const String &message) {
    message_ = normalizeText(message);
    if (message_.length() == 0) {
      message_ = "OK";
    }
    contentWidth_ = messagePixelWidth();
  }

  void setMessage(const char *message) {
    setMessage(String(message == nullptr ? "" : message));
  }

  void setColor(uint32_t colorValue) {
    colorValue_ = colorValue;
  }

  void setTimings(uint16_t stepIntervalMs, uint16_t pauseAfterScrollMs) {
    stepIntervalMs_ = stepIntervalMs;
    pauseAfterScrollMs_ = pauseAfterScrollMs;
  }

  void begin(LedMatrixCore &matrix) {
    scrollOffset_ = matrix.width();
    lastStepAtMs_ = 0;
    pauseStartedAtMs_ = 0;
    waitingPause_ = false;
    completedCycle_ = false;
    render(matrix);
  }

  void update(LedMatrixCore &matrix) {
    const uint32_t now = millis();

    if (waitingPause_) {
      if ((now - pauseStartedAtMs_) >= pauseAfterScrollMs_) {
        completedCycle_ = true;
        waitingPause_ = false;
      }
      return;
    }

    if (lastStepAtMs_ != 0 && (now - lastStepAtMs_) < stepIntervalMs_) {
      return;
    }

    lastStepAtMs_ = now;
    render(matrix);

    if ((scrollOffset_ + contentWidth_) <= 0) {
      waitingPause_ = true;
      pauseStartedAtMs_ = now;
      return;
    }

    scrollOffset_--;
  }

  bool consumeCompletedCycle() {
    const bool hadCompletedCycle = completedCycle_;
    completedCycle_ = false;
    return hadCompletedCycle;
  }

  const String &message() const {
    return message_;
  }

 private:
  static bool isAllowedPunctuation(char character) {
    switch (character) {
      case ' ':
      case '!':
      case '\'':
      case ',':
      case '-':
      case '.':
      case '/':
      case ':':
      case '?':
        return true;
      default:
        return false;
    }
  }

  static char mapUtf8Accent(uint8_t firstByte, uint8_t secondByte) {
    if (firstByte != 0xC3) {
      return '\0';
    }

    switch (secondByte) {
      case 0x80:
      case 0x81:
      case 0x82:
      case 0x83:
      case 0x84:
      case 0xA0:
      case 0xA1:
      case 0xA2:
      case 0xA3:
      case 0xA4:
        return 'A';
      case 0x87:
      case 0xA7:
        return 'C';
      case 0x88:
      case 0x89:
      case 0x8A:
      case 0xA8:
      case 0xA9:
      case 0xAA:
        return 'E';
      case 0x8D:
      case 0x8E:
      case 0xAD:
      case 0xAE:
        return 'I';
      case 0x93:
      case 0x94:
      case 0x95:
      case 0x96:
      case 0xB3:
      case 0xB4:
      case 0xB5:
      case 0xB6:
        return 'O';
      case 0x9A:
      case 0x9C:
      case 0xBA:
      case 0xBC:
        return 'U';
      default:
        return '\0';
    }
  }

  static String normalizeText(const String &rawMessage) {
    String normalized;
    normalized.reserve(rawMessage.length());

    bool previousWasSpace = true;

    for (size_t index = 0; index < rawMessage.length(); index++) {
      char current = rawMessage[index];

      if (((uint8_t)current) >= 0x80 && (index + 1) < rawMessage.length()) {
        const char mapped = mapUtf8Accent(
          (uint8_t)current,
          (uint8_t)rawMessage[index + 1]
        );
        if (mapped != '\0') {
          current = mapped;
          index++;
        } else {
          current = ' ';
        }
      }

      if (current >= 'a' && current <= 'z') {
        current = current - 'a' + 'A';
      }

      const bool allowed = (
        (current >= 'A' && current <= 'Z') ||
        (current >= '0' && current <= '9') ||
        isAllowedPunctuation(current)
      );

      if (!allowed) {
        current = ' ';
      }

      if (current == ' ') {
        if (previousWasSpace) {
          continue;
        }
        previousWasSpace = true;
      } else {
        previousWasSpace = false;
      }

      normalized += current;
    }

    normalized.trim();
    return normalized;
  }

  int16_t messagePixelWidth() const {
    if (message_.length() == 0) {
      return 0;
    }

    return (
      message_.length() * PixelFont5x7::glyphWidth() +
      (message_.length() - 1) * PixelFont5x7::glyphSpacing()
    );
  }

  void render(LedMatrixCore &matrix) {
    matrix.clear();

    const int8_t yOffset = (matrix.height() - PixelFont5x7::glyphHeight()) / 2;
    const uint8_t glyphWidth = PixelFont5x7::glyphWidth();
    const uint8_t glyphSpacing = PixelFont5x7::glyphSpacing();

    for (size_t index = 0; index < message_.length(); index++) {
      const uint8_t *glyph = PixelFont5x7::glyphFor(message_[index]);
      const int16_t glyphStartX = scrollOffset_ + index * (glyphWidth + glyphSpacing);

      for (uint8_t column = 0; column < glyphWidth; column++) {
        const int16_t x = glyphStartX + column;
        if (x < 0 || x >= matrix.width()) {
          continue;
        }

        const uint8_t bits = glyph[column];
        for (uint8_t row = 0; row < PixelFont5x7::glyphHeight(); row++) {
          if (bits & (1 << row)) {
            matrix.setXY(x, yOffset + row, colorValue_);
          }
        }
      }
    }

    matrix.show();
  }

  String message_ = "OK";
  uint32_t colorValue_ = 0;
  uint16_t stepIntervalMs_;
  uint16_t pauseAfterScrollMs_;
  uint32_t lastStepAtMs_ = 0;
  uint32_t pauseStartedAtMs_ = 0;
  int16_t scrollOffset_ = 0;
  int16_t contentWidth_ = 0;
  bool waitingPause_ = false;
  bool completedCycle_ = false;
};
