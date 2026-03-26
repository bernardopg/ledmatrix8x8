#pragma once

#include <Arduino.h>

#include <LedMatrixCore.h>
#include <LedMatrixEffect.h>

struct CatAnimationFrame {
  uint8_t body[8];
  uint8_t detail[8];
  uint8_t eyes[8];
};

static const CatAnimationFrame kCatAnimationFrames[] = {
  {
    {0x81, 0xC3, 0xFF, 0xFF, 0xFF, 0x7E, 0x3C, 0x00},
    {0x00, 0x00, 0x00, 0x00, 0x18, 0x24, 0x00, 0x00},
    {0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00}
  }
};

class CatAnimationEffect : public LedMatrixEffect {
 public:
  explicit CatAnimationEffect(uint16_t frameIntervalMs)
      : frameIntervalMs_(frameIntervalMs) {}

  const char *name() const override {
    return "CAT ONLY";
  }

  void begin(LedMatrixCore &matrix) override {
    lastFrameAtMs_ = 0;
    frameIndex_ = 0;
    drawFrame(matrix, frameIndex_);
  }

  void update(LedMatrixCore &matrix) override {
    const uint32_t now = millis();
    if (lastFrameAtMs_ != 0 && (now - lastFrameAtMs_) < frameIntervalMs_) {
      return;
    }

    lastFrameAtMs_ = now;
    drawFrame(matrix, frameIndex_);
    frameIndex_ = (frameIndex_ + 1) % frameCount();
  }

 private:
  static size_t frameCount() {
    return sizeof(kCatAnimationFrames) / sizeof(kCatAnimationFrames[0]);
  }

  void drawFrame(LedMatrixCore &matrix, size_t index) {
    const uint32_t bodyColor = matrix.color(255, 140, 0);
    const uint32_t detailColor = matrix.color(100, 100, 100);
    const uint32_t eyeColor = matrix.color(0, 39, 255);

    matrix.clear();
    for (uint8_t y = 0; y < 8; y++) {
      matrix.drawMaskRow(y, kCatAnimationFrames[index].body[y], bodyColor);
      matrix.drawMaskRow(y, kCatAnimationFrames[index].detail[y], detailColor);
      matrix.drawMaskRow(y, kCatAnimationFrames[index].eyes[y], eyeColor);
    }
    matrix.show();
  }

  uint16_t frameIntervalMs_;
  uint32_t lastFrameAtMs_ = 0;
  size_t frameIndex_ = 0;
};
