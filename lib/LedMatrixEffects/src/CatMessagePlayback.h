#pragma once

#include <Arduino.h>

#include <CatAnimation.h>
#include <LedMatrixEffect.h>
#include <LedMatrixIcons.h>
#include <LedMatrixTextTypes.h>
#include <TextMarquee.h>

class CatMessagePlaybackEffect : public LedMatrixEffect {
 public:
  CatMessagePlaybackEffect(
    uint16_t catFrameIntervalMs,
    uint8_t catLoops,
    uint16_t textStepIntervalMs,
    uint16_t textPauseMs,
    const LedMatrixTextMessage *messages,
    size_t messageCount
  )
      : cat_(catFrameIntervalMs),
        marquee_(textStepIntervalMs, textPauseMs),
        catFrameIntervalMs_(catFrameIntervalMs),
        catLoops_(catLoops),
        messages_(messages),
        messageCount_(messageCount) {}

  const char *name() const override {
    return "CAT + MARQUEE";
  }

  void begin(LedMatrixCore &matrix) override {
    currentMessageIndex_ = 0;
    state_ = PlaybackState::kCat;
    stateStartedAtMs_ = millis();
    cat_.begin(matrix);
  }

  void update(LedMatrixCore &matrix) override {
    if (!overrideEnabled_ && (messageCount_ == 0 || messages_ == nullptr)) {
      cat_.update(matrix);
      return;
    }

    switch (state_) {
      case PlaybackState::kCat:
        cat_.update(matrix);
        if ((millis() - stateStartedAtMs_) >= catDisplayDurationMs()) {
          beginCurrentPlayback(matrix);
        }
        break;

      case PlaybackState::kIcon:
        renderIcon(matrix);
        if ((millis() - stateStartedAtMs_) >= iconDisplayDurationMs_) {
          beginCurrentTextMessage(matrix);
        }
        break;

      case PlaybackState::kMessage:
        marquee_.update(matrix);
        if (marquee_.consumeCompletedCycle()) {
          if (!overrideEnabled_ && messageCount_ > 0) {
            currentMessageIndex_ = (currentMessageIndex_ + 1) % messageCount_;
          }
          restartCat(matrix);
        }
        break;
    }
  }

  bool supportsOverrides() const override {
    return true;
  }

  void showOverrideMessage(
    LedMatrixCore &matrix,
    const String &message,
    uint8_t red,
    uint8_t green,
    uint8_t blue
  ) {
    overrideText_ = message;
    overrideColor_.text = overrideText_.c_str();
    overrideColor_.red = red;
    overrideColor_.green = green;
    overrideColor_.blue = blue;
    overrideEnabled_ = true;
    activeIcon_ = nullptr;

    marquee_.setMessage(overrideText_);
    marquee_.setColor(matrix.color(red, green, blue));
    marquee_.begin(matrix);
    state_ = PlaybackState::kMessage;
  }

  void showIconMessage(
    LedMatrixCore &matrix,
    const LedMatrixIconDefinition &icon,
    const String &message,
    uint8_t red,
    uint8_t green,
    uint8_t blue
  ) override {
    overrideText_ = message.length() > 0 ? message : icon.label;
    overrideColor_.text = overrideText_.c_str();
    overrideColor_.red = red;
    overrideColor_.green = green;
    overrideColor_.blue = blue;
    overrideEnabled_ = true;
    activeIcon_ = &icon;
    beginIcon(matrix);
  }

  void clearOverrideMessage(LedMatrixCore &matrix) override {
    overrideEnabled_ = false;
    overrideText_ = "";
    overrideColor_.text = nullptr;
    activeIcon_ = nullptr;
    restartCat(matrix);
  }

  bool hasOverrideMessage() const override {
    return overrideEnabled_;
  }

 private:
  enum class PlaybackState {
    kCat,
    kIcon,
    kMessage
  };

  uint32_t catDisplayDurationMs() const {
    return static_cast<uint32_t>(catLoops_) *
           static_cast<uint32_t>(CatAnimationEffect::frameCount()) *
           static_cast<uint32_t>(catFrameIntervalMs_);
  }

  void restartCat(LedMatrixCore &matrix) {
    state_ = PlaybackState::kCat;
    stateStartedAtMs_ = millis();
    cat_.begin(matrix);
  }

  void beginIcon(LedMatrixCore &matrix) {
    state_ = PlaybackState::kIcon;
    stateStartedAtMs_ = millis();
    renderIcon(matrix);
  }

  void renderIcon(LedMatrixCore &matrix) {
    if (activeIcon_ == nullptr) {
      beginCurrentTextMessage(matrix);
      return;
    }

    matrix.clear();
    const uint32_t colorValue = matrix.color(
      overrideColor_.red,
      overrideColor_.green,
      overrideColor_.blue
    );
    for (uint8_t y = 0; y < 8; y++) {
      matrix.drawMaskRow(y, activeIcon_->rows[y], colorValue);
    }
    matrix.show();
  }

  void beginCurrentPlayback(LedMatrixCore &matrix) {
    if (overrideEnabled_ && activeIcon_ != nullptr) {
      beginIcon(matrix);
      return;
    }
    beginCurrentTextMessage(matrix);
  }

  void beginCurrentTextMessage(LedMatrixCore &matrix) {
    const LedMatrixTextMessage &message =
      overrideEnabled_ ? overrideColor_ : messages_[currentMessageIndex_];

    marquee_.setMessage(message.text);
    marquee_.setColor(matrix.color(message.red, message.green, message.blue));
    marquee_.begin(matrix);
    state_ = PlaybackState::kMessage;
  }

  CatAnimationEffect cat_;
  TextMarquee marquee_;
  uint16_t catFrameIntervalMs_ = 0;
  uint8_t catLoops_ = 1;
  const LedMatrixTextMessage *messages_ = nullptr;
  size_t messageCount_ = 0;
  size_t currentMessageIndex_ = 0;
  uint32_t stateStartedAtMs_ = 0;
  PlaybackState state_ = PlaybackState::kCat;
  String overrideText_;
  LedMatrixTextMessage overrideColor_ = {nullptr, 255, 255, 255};
  const LedMatrixIconDefinition *activeIcon_ = nullptr;
  uint16_t iconDisplayDurationMs_ = 1200;
  bool overrideEnabled_ = false;
};
