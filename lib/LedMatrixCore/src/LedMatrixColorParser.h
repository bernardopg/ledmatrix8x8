#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

struct LedMatrixRgb {
  uint8_t red = 0;
  uint8_t green = 0;
  uint8_t blue = 0;
};

inline bool ledMatrixIsSpace(char value) {
  return value == ' ' || value == '\t' || value == '\r' || value == '\n';
}

inline void ledMatrixTrimBounds(const char *&begin, const char *&end) {
  while (begin < end && ledMatrixIsSpace(*begin)) {
    begin++;
  }
  while (end > begin && ledMatrixIsSpace(*(end - 1))) {
    end--;
  }
}

inline bool ledMatrixParseByteComponent(
  const char *begin,
  const char *end,
  uint8_t &result
) {
  ledMatrixTrimBounds(begin, end);
  if (begin == end) {
    return false;
  }

  uint16_t parsed = 0;
  for (const char *cursor = begin; cursor < end; cursor++) {
    if (*cursor < '0' || *cursor > '9') {
      return false;
    }
    parsed = static_cast<uint16_t>((parsed * 10) + (*cursor - '0'));
    if (parsed > 255) {
      return false;
    }
  }

  result = static_cast<uint8_t>(parsed);
  return true;
}

inline bool parseLedMatrixRgbTriplet(const char *payload, LedMatrixRgb &result) {
  if (payload == nullptr) {
    return false;
  }

  const char *begin = payload;
  const char *end = payload + std::strlen(payload);
  ledMatrixTrimBounds(begin, end);

  const char *firstComma = nullptr;
  const char *secondComma = nullptr;
  for (const char *cursor = begin; cursor < end; cursor++) {
    if (*cursor != ',') {
      continue;
    }
    if (firstComma == nullptr) {
      firstComma = cursor;
    } else if (secondComma == nullptr) {
      secondComma = cursor;
    } else {
      return false;
    }
  }

  if (firstComma == nullptr || secondComma == nullptr) {
    return false;
  }

  LedMatrixRgb parsed{};
  if (!ledMatrixParseByteComponent(begin, firstComma, parsed.red) ||
      !ledMatrixParseByteComponent(firstComma + 1, secondComma, parsed.green) ||
      !ledMatrixParseByteComponent(secondComma + 1, end, parsed.blue)) {
    return false;
  }

  result = parsed;
  return true;
}

inline bool ledMatrixParseHexNibble(char value, uint8_t &result) {
  if (value >= '0' && value <= '9') {
    result = static_cast<uint8_t>(value - '0');
    return true;
  }

  if (value >= 'a' && value <= 'f') {
    result = static_cast<uint8_t>(value - 'a' + 10);
    return true;
  }

  if (value >= 'A' && value <= 'F') {
    result = static_cast<uint8_t>(value - 'A' + 10);
    return true;
  }

  return false;
}

inline bool parseLedMatrixHexColor(const char *payload, LedMatrixRgb &result) {
  if (payload == nullptr) {
    return false;
  }

  const char *begin = payload;
  const char *end = payload + std::strlen(payload);
  ledMatrixTrimBounds(begin, end);

  if (begin < end && *begin == '#') {
    begin++;
  }

  if ((end - begin) != 6) {
    return false;
  }

  uint8_t nibbles[6] = {0, 0, 0, 0, 0, 0};
  for (std::size_t index = 0; index < 6; index++) {
    if (!ledMatrixParseHexNibble(begin[index], nibbles[index])) {
      return false;
    }
  }

  result.red = static_cast<uint8_t>((nibbles[0] << 4) | nibbles[1]);
  result.green = static_cast<uint8_t>((nibbles[2] << 4) | nibbles[3]);
  result.blue = static_cast<uint8_t>((nibbles[4] << 4) | nibbles[5]);
  return true;
}

inline bool ledMatrixEqualsLiteral(
  const char *begin,
  const char *end,
  const char *literal
) {
  const std::size_t length = static_cast<std::size_t>(end - begin);
  return std::strlen(literal) == length && std::strncmp(begin, literal, length) == 0;
}

inline bool parseLedMatrixHomeAssistantColor(
  const char *payload,
  LedMatrixRgb &result
) {
  if (payload == nullptr) {
    return false;
  }

  const char *begin = payload;
  const char *end = payload + std::strlen(payload);
  ledMatrixTrimBounds(begin, end);

  if (begin == end ||
      ledMatrixEqualsLiteral(begin, end, "unknown") ||
      ledMatrixEqualsLiteral(begin, end, "unavailable")) {
    return false;
  }

  return parseLedMatrixRgbTriplet(payload, result) ||
         parseLedMatrixHexColor(payload, result);
}
