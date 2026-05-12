#pragma once

#include <cstdint>

enum class LedMatrixMessageSource : uint8_t {
  Config = 0,
  HomeAssistant = 1,
  Serial = 2,
};

inline constexpr LedMatrixMessageSource resolveLedMatrixMessageSource(
  bool serialActive,
  bool homeAssistantActive
) {
  return serialActive
    ? LedMatrixMessageSource::Serial
    : (homeAssistantActive
        ? LedMatrixMessageSource::HomeAssistant
        : LedMatrixMessageSource::Config);
}

inline const char *ledMatrixMessageSourceLabel(
  LedMatrixMessageSource source
) {
  switch (source) {
    case LedMatrixMessageSource::Serial:
      return "SERIAL";
    case LedMatrixMessageSource::HomeAssistant:
      return "HOME_ASSISTANT";
    case LedMatrixMessageSource::Config:
    default:
      return "CONFIG";
  }
}
