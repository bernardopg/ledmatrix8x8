#pragma once

/*
 * LED Matrix 8x8 - app selecionando um efeito importavel.
 *
 * O gato continua como biblioteca independente e o letreiro vive em outra
 * biblioteca. Este app apenas orquestra o playback entre os dois.
 */

#include <Arduino.h>

#include <CatMessagePlayback.h>
#include <HomeAssistantTextClient.h>
#include <LedMatrixCore.h>

#if __has_include("ledmatrix8x8_secrets.h")
#include "ledmatrix8x8_secrets.h"
#else
#include "ledmatrix8x8_secrets.example.h"
#endif

#include "generated/project_content.h"

#define PIN_LED       38
#define MATRIX_WIDTH  8
#define MATRIX_HEIGHT 8

#define MATRIX_ZIGZAG 0
#define ORIGIN_BOTTOM 0

static LedMatrixCore matrix(
  MATRIX_WIDTH,
  MATRIX_HEIGHT,
  PIN_LED,
  MATRIX_ZIGZAG,
  ORIGIN_BOTTOM
);

static CatMessagePlaybackEffect currentEffect(
  PROJECT_CAT_FRAME_MS,
  PROJECT_CAT_LOOPS,
  PROJECT_SCROLL_STEP_MS,
  PROJECT_MESSAGE_PAUSE_MS,
  PROJECT_MESSAGES,
  PROJECT_MESSAGE_COUNT
);

static HomeAssistantTextClient homeAssistantClient(
  LEDMATRIX_WIFI_SSID,
  LEDMATRIX_WIFI_PASSWORD,
  LEDMATRIX_WIFI_USE_STATIC_IP,
  LEDMATRIX_WIFI_STATIC_IP,
  LEDMATRIX_WIFI_GATEWAY,
  LEDMATRIX_WIFI_SUBNET,
  LEDMATRIX_WIFI_DNS1,
  LEDMATRIX_WIFI_DNS2,
  LEDMATRIX_HA_BASE_URL,
  LEDMATRIX_HA_ACCESS_TOKEN,
  LEDMATRIX_HA_ENTITY_ID,
  PROJECT_HA_POLL_MS,
  true
);

static HomeAssistantTextClient homeAssistantColorClient(
  LEDMATRIX_WIFI_SSID,
  LEDMATRIX_WIFI_PASSWORD,
  LEDMATRIX_WIFI_USE_STATIC_IP,
  LEDMATRIX_WIFI_STATIC_IP,
  LEDMATRIX_WIFI_GATEWAY,
  LEDMATRIX_WIFI_SUBNET,
  LEDMATRIX_WIFI_DNS1,
  LEDMATRIX_WIFI_DNS2,
  LEDMATRIX_HA_BASE_URL,
  LEDMATRIX_HA_ACCESS_TOKEN,
  LEDMATRIX_HA_COLOR_ENTITY_ID,
  PROJECT_HA_POLL_MS,
  false
);

static String serialLineBuffer;
static uint8_t serialOverrideRed = 255;
static uint8_t serialOverrideGreen = 255;
static uint8_t serialOverrideBlue = 255;
static bool manualOverrideActive = false;
static bool homeAssistantOverrideActive = false;
static String homeAssistantMessage;
static uint8_t homeAssistantRed = PROJECT_HA_COLOR_RED;
static uint8_t homeAssistantGreen = PROJECT_HA_COLOR_GREEN;
static uint8_t homeAssistantBlue = PROJECT_HA_COLOR_BLUE;

inline void printSerialHelp() {
  Serial.println("Comandos:");
  Serial.println("  TEXT:mensagem livre");
  Serial.println("  COLOR:r,g,b");
  Serial.println("  CLEAR");
  Serial.println("  STATUS");
  Serial.println("  HELP");
}

inline bool parseColorCommand(const String &payload) {
  const int firstComma = payload.indexOf(',');
  const int secondComma = payload.indexOf(',', firstComma + 1);

  if (firstComma < 0 || secondComma < 0) {
    return false;
  }

  const int red = payload.substring(0, firstComma).toInt();
  const int green = payload.substring(firstComma + 1, secondComma).toInt();
  const int blue = payload.substring(secondComma + 1).toInt();

  if (red < 0 || red > 255 || green < 0 || green > 255 || blue < 0 || blue > 255) {
    return false;
  }

  serialOverrideRed = static_cast<uint8_t>(red);
  serialOverrideGreen = static_cast<uint8_t>(green);
  serialOverrideBlue = static_cast<uint8_t>(blue);
  return true;
}

inline bool parseRgbString(
  const String &payload,
  uint8_t &red,
  uint8_t &green,
  uint8_t &blue
) {
  const int firstComma = payload.indexOf(',');
  const int secondComma = payload.indexOf(',', firstComma + 1);

  if (firstComma < 0 || secondComma < 0) {
    return false;
  }

  const int parsedRed = payload.substring(0, firstComma).toInt();
  const int parsedGreen = payload.substring(firstComma + 1, secondComma).toInt();
  const int parsedBlue = payload.substring(secondComma + 1).toInt();

  if (parsedRed < 0 || parsedRed > 255 ||
      parsedGreen < 0 || parsedGreen > 255 ||
      parsedBlue < 0 || parsedBlue > 255) {
    return false;
  }

  red = static_cast<uint8_t>(parsedRed);
  green = static_cast<uint8_t>(parsedGreen);
  blue = static_cast<uint8_t>(parsedBlue);
  return true;
}

inline bool parseHexNibble(char value, uint8_t &result) {
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

inline bool parseHexColor(
  const String &payload,
  uint8_t &red,
  uint8_t &green,
  uint8_t &blue
) {
  String value = payload;
  value.trim();

  if (value.startsWith("#")) {
    value = value.substring(1);
  }

  if (value.length() != 6) {
    return false;
  }

  uint8_t nibbles[6];
  for (size_t index = 0; index < 6; index++) {
    if (!parseHexNibble(value[index], nibbles[index])) {
      return false;
    }
  }

  red = static_cast<uint8_t>((nibbles[0] << 4) | nibbles[1]);
  green = static_cast<uint8_t>((nibbles[2] << 4) | nibbles[3]);
  blue = static_cast<uint8_t>((nibbles[4] << 4) | nibbles[5]);
  return true;
}

inline bool parseHomeAssistantColor(
  String payload,
  uint8_t &red,
  uint8_t &green,
  uint8_t &blue
) {
  payload.trim();
  if (payload.length() == 0 || payload == "unknown" || payload == "unavailable") {
    return false;
  }

  return parseRgbString(payload, red, green, blue) ||
         parseHexColor(payload, red, green, blue);
}

inline void processSerialCommand(String command) {
  command.trim();
  if (command.length() == 0) {
    return;
  }

  if (command.startsWith("TEXT:")) {
    const String message = command.substring(5);
    manualOverrideActive = true;
    currentEffect.showOverrideMessage(
      matrix,
      message,
      serialOverrideRed,
      serialOverrideGreen,
      serialOverrideBlue
    );
    Serial.print("Override ativo: ");
    Serial.println(message);
    return;
  }

  if (command.startsWith("COLOR:")) {
    const String payload = command.substring(6);
    if (parseColorCommand(payload)) {
      Serial.print("Cor do override: ");
      Serial.print(serialOverrideRed);
      Serial.print(",");
      Serial.print(serialOverrideGreen);
      Serial.print(",");
      Serial.println(serialOverrideBlue);
    } else {
      Serial.println("Uso: COLOR:r,g,b");
    }
    return;
  }

  if (command == "CLEAR") {
    manualOverrideActive = false;
    if (homeAssistantOverrideActive) {
      currentEffect.showOverrideMessage(
        matrix,
        homeAssistantMessage,
        PROJECT_HA_COLOR_RED,
        PROJECT_HA_COLOR_GREEN,
        PROJECT_HA_COLOR_BLUE
      );
      Serial.println("Override manual limpo; Home Assistant reassumiu");
    } else {
      currentEffect.clearOverrideMessage(matrix);
      Serial.println("Override limpo; voltando ao config.yaml");
    }
    return;
  }

  if (command == "STATUS") {
    Serial.print("Override efetivo: ");
    Serial.println(currentEffect.hasOverrideMessage() ? "SIM" : "NAO");
    Serial.print("Override manual: ");
    Serial.println(manualOverrideActive ? "SIM" : "NAO");
    Serial.print("Override Home Assistant: ");
    Serial.println(homeAssistantOverrideActive ? "SIM" : "NAO");
    Serial.print("Cor serial atual: ");
    Serial.print(serialOverrideRed);
    Serial.print(",");
    Serial.print(serialOverrideGreen);
    Serial.print(",");
    Serial.println(serialOverrideBlue);
    return;
  }

  if (command == "HELP") {
    printSerialHelp();
    return;
  }

  Serial.print("Comando desconhecido: ");
  Serial.println(command);
  printSerialHelp();
}

inline void handleSerialInput() {
  while (Serial.available() > 0) {
    const char incoming = static_cast<char>(Serial.read());

    if (incoming == '\r' || incoming == '\n') {
      if (serialLineBuffer.length() > 0) {
        processSerialCommand(serialLineBuffer);
        serialLineBuffer = "";
      }
      continue;
    }

    if (serialLineBuffer.length() < 160) {
      serialLineBuffer += incoming;
    }
  }
}

inline void handleHomeAssistant() {
  homeAssistantClient.update();
  homeAssistantColorClient.update();

  bool colorHasMessage = false;
  String colorMessage;
  if (homeAssistantColorClient.consumeStateChange(colorHasMessage, colorMessage)) {
    uint8_t red = PROJECT_HA_COLOR_RED;
    uint8_t green = PROJECT_HA_COLOR_GREEN;
    uint8_t blue = PROJECT_HA_COLOR_BLUE;

    if (colorHasMessage && parseHomeAssistantColor(colorMessage, red, green, blue)) {
      homeAssistantRed = red;
      homeAssistantGreen = green;
      homeAssistantBlue = blue;
      Serial.print("Home Assistant atualizou cor: ");
      Serial.print(homeAssistantRed);
      Serial.print(",");
      Serial.print(homeAssistantGreen);
      Serial.print(",");
      Serial.println(homeAssistantBlue);
    } else {
      homeAssistantRed = PROJECT_HA_COLOR_RED;
      homeAssistantGreen = PROJECT_HA_COLOR_GREEN;
      homeAssistantBlue = PROJECT_HA_COLOR_BLUE;
      Serial.println("Home Assistant cor invalida/vazia; usando cor padrao");
    }

    if (homeAssistantOverrideActive && !manualOverrideActive && homeAssistantMessage.length() > 0) {
      currentEffect.showOverrideMessage(
        matrix,
        homeAssistantMessage,
        homeAssistantRed,
        homeAssistantGreen,
        homeAssistantBlue
      );
    }
  }

  bool hasMessage = false;
  String message;
  if (!homeAssistantClient.consumeStateChange(hasMessage, message)) {
    return;
  }

  homeAssistantOverrideActive = hasMessage;
  homeAssistantMessage = hasMessage ? message : "";

  if (manualOverrideActive) {
    Serial.println("Home Assistant atualizou, mas override manual continua prioritario");
    return;
  }

  if (hasMessage) {
    currentEffect.showOverrideMessage(
      matrix,
      message,
      homeAssistantRed,
      homeAssistantGreen,
      homeAssistantBlue
    );
  } else {
    currentEffect.clearOverrideMessage(matrix);
  }
}

inline void ledmatrix8x8Setup() {
  Serial.begin(115200);
  delay(1200);

  matrix.begin(PROJECT_BRIGHTNESS);

  Serial.println();
  Serial.println("==== LEDMATRIX8X8 ====");
  Serial.print("Efeito atual: ");
  Serial.println(currentEffect.name());
  Serial.print("Brightness: ");
  Serial.println(PROJECT_BRIGHTNESS);
  Serial.print("Target HA base URL: ");
  Serial.println(LEDMATRIX_HA_BASE_URL);
  printSerialHelp();

  currentEffect.begin(matrix);
  homeAssistantClient.begin();
  homeAssistantColorClient.begin();
}

inline void ledmatrix8x8Loop() {
  handleSerialInput();
  handleHomeAssistant();
  currentEffect.update(matrix);
}
