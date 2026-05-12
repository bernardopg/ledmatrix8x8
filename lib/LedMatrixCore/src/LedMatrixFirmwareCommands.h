#pragma once

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>

inline bool parseLedMatrixBrightnessValue(const char *value, uint8_t &brightness) {
  if (value == nullptr || value[0] == '\0') {
    return false;
  }

  char *end = nullptr;
  const long parsed = std::strtol(value, &end, 10);
  if (end == value || *end != '\0' || parsed < 0 || parsed > 255) {
    return false;
  }

  brightness = static_cast<uint8_t>(parsed);
  return true;
}

enum class LedMatrixEffectMode {
  kCatOnly,
  kCatMessagePlayback,
};

inline bool parseLedMatrixEffectMode(const char *value, LedMatrixEffectMode &mode) {
  if (value == nullptr || value[0] == '\0') {
    return false;
  }

  std::string normalized(value);
  for (char &ch : normalized) {
    if (ch >= 'A' && ch <= 'Z') {
      ch = static_cast<char>(ch - 'A' + 'a');
    }
  }

  if (normalized == "cat" || normalized == "cat only" || normalized == "cat-only") {
    mode = LedMatrixEffectMode::kCatOnly;
    return true;
  }

  if (normalized == "cat + marquee" || normalized == "cat+marquee" ||
      normalized == "marquee" || normalized == "playback" || normalized == "message") {
    mode = LedMatrixEffectMode::kCatMessagePlayback;
    return true;
  }

  return false;
}

inline std::string formatLedMatrixHomeAssistantErrorSummary(int httpStatus, const char *jsonError) {
  const bool hasHttpError = httpStatus > 0 && httpStatus != 200;
  const bool hasJsonError = jsonError != nullptr && jsonError[0] != '\0';

  if (!hasHttpError && !hasJsonError) {
    return "<nenhum>";
  }

  std::string summary;
  if (hasHttpError) {
    summary += "HTTP ";
    summary += std::to_string(httpStatus);
  }
  if (hasJsonError) {
    if (!summary.empty()) {
      summary += " / ";
    }
    summary += "JSON ";
    summary += jsonError;
  }
  return summary;
}
