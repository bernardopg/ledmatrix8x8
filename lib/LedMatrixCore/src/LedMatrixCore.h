#pragma once

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

class LedMatrixCore {
 public:
  LedMatrixCore(
    uint16_t width,
    uint16_t height,
    uint8_t pin,
    bool zigzag,
    bool originBottom
  )
      : width_(width),
        height_(height),
        zigzag_(zigzag),
        originBottom_(originBottom),
        pixels_(width * height, pin, NEO_GRB + NEO_KHZ800) {}

  void begin(uint8_t brightness) {
    pixels_.begin();
    pixels_.setBrightness(brightness);
    pixels_.clear();
    pixels_.show();
  }

  void setBrightness(uint8_t brightness) {
    pixels_.setBrightness(brightness);
  }

  void clear() {
    pixels_.clear();
  }

  void show() {
    pixels_.show();
  }

  uint16_t width() const {
    return width_;
  }

  uint16_t height() const {
    return height_;
  }

  uint32_t color(uint8_t red, uint8_t green, uint8_t blue) {
    return pixels_.gamma32(pixels_.Color(red, green, blue));
  }

  void setXY(uint8_t x, uint8_t y, uint32_t colorValue) {
    if (x >= width_ || y >= height_) {
      return;
    }

    pixels_.setPixelColor(xyToIndex(x, y), colorValue);
  }

  void drawMaskRow(uint8_t y, uint8_t rowBits, uint32_t colorValue) {
    for (uint8_t x = 0; x < width_; x++) {
      if (rowBits & (0x80 >> x)) {
        setXY(x, y, colorValue);
      }
    }
  }

 private:
  uint16_t xyToIndex(uint8_t x, uint8_t y) const {
    if (originBottom_) {
      y = (height_ - 1) - y;
    }

    if (zigzag_ && (y & 0x01)) {
      x = (width_ - 1) - x;
    }

    return (uint16_t)y * width_ + x;
  }

  uint16_t width_;
  uint16_t height_;
  bool zigzag_;
  bool originBottom_;
  Adafruit_NeoPixel pixels_;
};
