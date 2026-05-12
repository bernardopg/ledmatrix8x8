#pragma once

#include <Arduino.h>

#include <LedMatrixCore.h>

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

  // Called by the app when CLEAR is received and no HA message is active.
  virtual void clearOverrideMessage(LedMatrixCore &matrix) {
    (void)matrix;
  }

  // Returns true if an override message is currently active.
  virtual bool hasOverrideMessage() const {
    return false;
  }
};
