#pragma once

#include <Arduino.h>

#include <LedMatrixCore.h>

class LedMatrixEffect {
 public:
  virtual ~LedMatrixEffect() = default;

  virtual const char *name() const = 0;
  virtual void begin(LedMatrixCore &matrix) = 0;
  virtual void update(LedMatrixCore &matrix) = 0;
};
