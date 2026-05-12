#pragma once

#include <Arduino.h>

#include <LedMatrixCore.h>

struct LedMatrixIconDefinition;

class LedMatrixEffect {
 public:
  virtual ~LedMatrixEffect() = default;

  virtual const char *name() const = 0;
  virtual void begin(LedMatrixCore &matrix) = 0;
  virtual void update(LedMatrixCore &matrix) = 0;

  // Returns true if this effect actively supports TEXT/CLEAR overrides.
  // The app checks this before calling showOverrideMessage to decide whether
  // to log a warning. Effects that do not override showOverrideMessage should
  // return false here so callers can react accordingly.
  virtual bool supportsOverrides() const {
    return false;
  }

  // Called by the app when a TEXT: serial command or HA message is received.
  // Effects that support overrides must override this method.
  virtual void showOverrideMessage(
    LedMatrixCore &matrix,
    const String &message,
    uint8_t red,
    uint8_t green,
    uint8_t blue
  ) {
    (void)matrix;
    (void)message;
    (void)red;
    (void)green;
    (void)blue;
  }

  // Called by the app when an ICON: command or HA ICON: prefix is received.
  // Effects that do not have a dedicated icon state can fall back to text.
  virtual void showIconMessage(
    LedMatrixCore &matrix,
    const LedMatrixIconDefinition &icon,
    const String &message,
    uint8_t red,
    uint8_t green,
    uint8_t blue
  ) {
    (void)icon;
    showOverrideMessage(matrix, message, red, green, blue);
  }

  // Called by the app when CLEAR is received and no HA message is active.
  virtual void clearOverrideMessage(LedMatrixCore &matrix) {
    (void)matrix;
  }

  // Returns true if an override message is currently active.
  virtual bool hasOverrideMessage() const {
    return false;
  }
};
