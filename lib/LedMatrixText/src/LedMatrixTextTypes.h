#pragma once

#include <Arduino.h>

struct LedMatrixTextMessage {
  const char *text;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};
