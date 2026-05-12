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
#include <LedMatrixColorParser.h>
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
  LedMatrixRgb parsed{};
  if (!parseLedMatrixRgbTriplet(payload.c_str(), parsed)) {
    return false;
  }

  serialOverrideRed = parsed.red;
  serialOverrideGreen = parsed.green;
  serialOverrideBlue = parsed.blue;
  return true;
}

inline bool parseHomeAssistantColor(
  const String &payload,
  uint8_t &red,
  uint8_t &green,
  uint8_t &blue
) {
  LedMatrixRgb parsed{};
  if (!parseLedMatrixHomeAssistantColor(payload.c_str(), parsed)) {
    return false;
  }

  red = parsed.red;
  green = parsed.green;
  blue = parsed.blue;
  return true;
}

inline void printYesNo(bool value) {
  Serial.println(value ? "SIM" : "NAO");
}

inline const char *currentMessageSource() {
  if (manualOverrideActive) {
    return "SERIAL";
  }
  if (homeAssistantOverrideActive) {
    return "HOME_ASSISTANT";
  }
  return "CONFIG";
}

inline void printLastPoll(const HomeAssistantTextClient &client) {
  if (client.lastPollAtMs() == 0) {
    Serial.println("NUNCA");
    return;
  }
  Serial.print(client.lastPollAtMs());
  Serial.println(" ms");
}

inline void printClientDiagnostics(
  const char *label,
  const HomeAssistantTextClient &client
) {
  Serial.print(label);
  Serial.print(" habilitado: ");
  printYesNo(client.enabled());
  Serial.print(label);
  Serial.print(" entity: ");
  Serial.println(client.entityId().length() > 0 ? client.entityId() : "<nao configurado>");
  Serial.print(label);
  Serial.print(" ultimo HTTP: ");
  Serial.println(client.lastHttpStatus());
  Serial.print(label);
  Serial.print(" ultimo poll: ");
  printLastPoll(client);
  Serial.print(label);
  Serial.print(" ultimo estado: ");
  Serial.println(client.lastState().length() > 0 ? client.lastState() : "<vazio>");
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
        homeAssistantRed,
        homeAssistantGreen,
        homeAssistantBlue
      );
      Serial.println("Override manual limpo; Home Assistant reassumiu");
    } else {
      currentEffect.clearOverrideMessage(matrix);
      Serial.println("Override limpo; voltando ao config.yaml");
    }
    return;
  }

  if (command == "STATUS") {
    Serial.println("---- STATUS LEDMATRIX8X8 ----");
    Serial.print("Fonte atual: ");
    Serial.println(currentMessageSource());
    Serial.print("Uptime: ");
    Serial.print(millis());
    Serial.println(" ms");
    Serial.print("Efeito atual: ");
    Serial.println(currentEffect.name());
    Serial.print("Brightness: ");
    Serial.println(PROJECT_BRIGHTNESS);
    Serial.print("Override efetivo: ");
    printYesNo(currentEffect.hasOverrideMessage());
    Serial.print("Override manual: ");
    printYesNo(manualOverrideActive);
    Serial.print("Override Home Assistant: ");
    printYesNo(homeAssistantOverrideActive);
    Serial.print("Cor serial atual: ");
    Serial.print(serialOverrideRed);
    Serial.print(",");
    Serial.print(serialOverrideGreen);
    Serial.print(",");
    Serial.println(serialOverrideBlue);
    Serial.print("Cor Home Assistant atual: ");
    Serial.print(homeAssistantRed);
    Serial.print(",");
    Serial.print(homeAssistantGreen);
    Serial.print(",");
    Serial.println(homeAssistantBlue);
    Serial.print("Mensagem Home Assistant ativa: ");
    Serial.println(homeAssistantMessage.length() > 0 ? homeAssistantMessage : "<vazia>");
    Serial.print("WiFi configurado: ");
    printYesNo(homeAssistantClient.wifiConfiguredForDiagnostics());
    Serial.print("WiFi gerenciado por cliente principal: ");
    printYesNo(homeAssistantClient.managesWifi());
    Serial.print("WiFi conectado: ");
    printYesNo(homeAssistantClient.wifiConnected());
    Serial.print("WiFi status: ");
    Serial.println(homeAssistantClient.wifiStatusText());
    Serial.print("WiFi IP: ");
    Serial.println(homeAssistantClient.localIpText());
    printClientDiagnostics("HA mensagem", homeAssistantClient);
    printClientDiagnostics("HA cor", homeAssistantColorClient);
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
