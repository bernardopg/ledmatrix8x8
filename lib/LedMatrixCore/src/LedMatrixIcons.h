#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

// 8x8 status glyphs. Each row uses bit 7 as the left-most pixel.
enum class LedMatrixIconId {
  kAgenda,
  kTask,
  kStatus,
  kFocus,
  kCall,
  kBreak,
  kError,
};

struct LedMatrixIconDefinition {
  LedMatrixIconId id;
  const char *name;
  const char *label;
  uint8_t defaultRed;
  uint8_t defaultGreen;
  uint8_t defaultBlue;
  uint8_t rows[8];
};

static constexpr LedMatrixIconDefinition LED_MATRIX_ICONS[] = {
  {
    LedMatrixIconId::kAgenda,
    "agenda",
    "AGENDA",
    0,
    180,
    255,
    {0x7E, 0x5A, 0x7E, 0x42, 0x5A, 0x42, 0x7E, 0x00},
  },
  {
    LedMatrixIconId::kTask,
    "task",
    "TASK",
    255,
    140,
    0,
    {0x7E, 0x42, 0x5A, 0x42, 0x52, 0x4C, 0x7E, 0x00},
  },
  {
    LedMatrixIconId::kStatus,
    "status",
    "STATUS",
    0,
    255,
    160,
    {0x3C, 0x42, 0x81, 0x91, 0x89, 0x42, 0x3C, 0x00},
  },
  {
    LedMatrixIconId::kFocus,
    "focus",
    "FOCO",
    255,
    140,
    0,
    {0x18, 0x24, 0x5A, 0xA5, 0xA5, 0x5A, 0x24, 0x18},
  },
  {
    LedMatrixIconId::kCall,
    "call",
    "CALL",
    0,
    180,
    255,
    {0x18, 0x24, 0x52, 0x4A, 0x52, 0x24, 0x18, 0x00},
  },
  {
    LedMatrixIconId::kBreak,
    "break",
    "BREAK",
    180,
    0,
    255,
    {0x3C, 0x42, 0x5A, 0x42, 0x3C, 0x18, 0x3C, 0x00},
  },
  {
    LedMatrixIconId::kError,
    "error",
    "ERROR",
    255,
    0,
    64,
    {0x18, 0x3C, 0x7E, 0xDB, 0xFF, 0x18, 0x18, 0x00},
  },
};

static constexpr size_t LED_MATRIX_ICON_COUNT =
  sizeof(LED_MATRIX_ICONS) / sizeof(LED_MATRIX_ICONS[0]);

inline const LedMatrixIconDefinition *ledMatrixIconById(LedMatrixIconId id) {
  for (size_t index = 0; index < LED_MATRIX_ICON_COUNT; index++) {
    if (LED_MATRIX_ICONS[index].id == id) {
      return &LED_MATRIX_ICONS[index];
    }
  }
  return nullptr;
}

inline const char *ledMatrixIconLabel(LedMatrixIconId id) {
  const LedMatrixIconDefinition *icon = ledMatrixIconById(id);
  return icon == nullptr ? "UNKNOWN" : icon->label;
}

inline const char *ledMatrixIconName(LedMatrixIconId id) {
  const LedMatrixIconDefinition *icon = ledMatrixIconById(id);
  return icon == nullptr ? "unknown" : icon->name;
}

inline std::string normalizeLedMatrixIconName(const char *value) {
  std::string normalized(value == nullptr ? "" : value);
  while (!normalized.empty() && normalized.front() == ' ') {
    normalized.erase(normalized.begin());
  }
  while (!normalized.empty() && normalized.back() == ' ') {
    normalized.pop_back();
  }

  for (char &ch : normalized) {
    if (ch >= 'A' && ch <= 'Z') {
      ch = static_cast<char>(ch - 'A' + 'a');
    } else if (ch == '_' || ch == ' ') {
      ch = '-';
    }
  }
  return normalized;
}

inline bool parseLedMatrixIconId(const char *value, LedMatrixIconId &iconId) {
  const std::string normalized = normalizeLedMatrixIconName(value);

  if (normalized == "agenda" || normalized == "calendar" || normalized == "calendario") {
    iconId = LedMatrixIconId::kAgenda;
    return true;
  }
  if (normalized == "task" || normalized == "todo" || normalized == "tarefa") {
    iconId = LedMatrixIconId::kTask;
    return true;
  }
  if (normalized == "status" || normalized == "ok" || normalized == "done") {
    iconId = LedMatrixIconId::kStatus;
    return true;
  }
  if (normalized == "focus" || normalized == "foco" || normalized == "target") {
    iconId = LedMatrixIconId::kFocus;
    return true;
  }
  if (normalized == "call" || normalized == "phone" || normalized == "ligacao") {
    iconId = LedMatrixIconId::kCall;
    return true;
  }
  if (normalized == "break" || normalized == "pause" || normalized == "coffee") {
    iconId = LedMatrixIconId::kBreak;
    return true;
  }
  if (normalized == "error" || normalized == "erro" || normalized == "alert") {
    iconId = LedMatrixIconId::kError;
    return true;
  }

  return false;
}

inline const char *ledMatrixIconCommandList() {
  return "agenda|task|status|focus|call|break|error";
}
