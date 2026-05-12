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
#include <LedMatrixFirmwareCommands.h>
#include <LedMatrixIcons.h>
#include <LedMatrixMessagePriority.h>

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

static CatAnimationEffect catOnlyEffect(PROJECT_CAT_FRAME_MS);
static CatMessagePlaybackEffect catMessageEffect(
  PROJECT_CAT_FRAME_MS,
  PROJECT_CAT_LOOPS,
  PROJECT_SCROLL_STEP_MS,
  PROJECT_MESSAGE_PAUSE_MS,
  PROJECT_MESSAGES,
  PROJECT_MESSAGE_COUNT
);
static LedMatrixEffectMode currentEffectMode = LedMatrixEffectMode::kCatMessagePlayback;
static LedMatrixEffect *currentEffect = &catMessageEffect;

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
static String serialOverrideMessage;
static uint8_t serialOverrideRed = 255;
static uint8_t serialOverrideGreen = 255;
static uint8_t serialOverrideBlue = 255;
static bool manualOverrideActive = false;
static bool homeAssistantOverrideActive = false;
static String homeAssistantMessage;
static const LedMatrixIconDefinition *serialOverrideIcon = nullptr;
static const LedMatrixIconDefinition *homeAssistantIcon = nullptr;
static uint8_t homeAssistantRed = PROJECT_HA_COLOR_RED;
static uint8_t homeAssistantGreen = PROJECT_HA_COLOR_GREEN;
static uint8_t homeAssistantBlue = PROJECT_HA_COLOR_BLUE;
static uint8_t runtimeBrightness = PROJECT_BRIGHTNESS;

struct LedMatrixOverridePayload {
  String message;
  const LedMatrixIconDefinition *icon = nullptr;
};

inline LedMatrixOverridePayload resolveLedMatrixOverridePayload(const String &rawMessage) {
  LedMatrixOverridePayload payload;
  payload.message = rawMessage;
  payload.icon = nullptr;
  if (rawMessage.length() < 6) {
    return payload;
  }

  String prefix = rawMessage.substring(0, 5);
  prefix.toUpperCase();
  if (prefix != "ICON:") {
    return payload;
  }

  String iconPayload = rawMessage.substring(5);
  const int separator = iconPayload.indexOf(':');
  String iconName = separator >= 0 ? iconPayload.substring(0, separator) : iconPayload;
  String iconMessage = separator >= 0 ? iconPayload.substring(separator + 1) : "";
  iconName.trim();
  iconMessage.trim();

  LedMatrixIconId iconId = LedMatrixIconId::kStatus;
  if (!parseLedMatrixIconId(iconName.c_str(), iconId)) {
    return payload;
  }

  const LedMatrixIconDefinition *icon = ledMatrixIconById(iconId);
  if (icon == nullptr) {
    return payload;
  }

  payload.icon = icon;
  payload.message = iconMessage.length() > 0 ? iconMessage : icon->label;
  return payload;
}

inline bool showOverridePayload(
  const String &message,
  const LedMatrixIconDefinition *icon,
  uint8_t red,
  uint8_t green,
  uint8_t blue,
  const char *unsupportedWarning
) {
  if (!currentEffect->supportsOverrides()) {
    Serial.println(unsupportedWarning);
    return false;
  }

  if (icon != nullptr) {
    currentEffect->showIconMessage(matrix, *icon, message, red, green, blue);
  } else {
    currentEffect->showOverrideMessage(matrix, message, red, green, blue);
  }
  return true;
}

inline void applyEffectMode(LedMatrixEffectMode mode) {
  currentEffectMode = mode;
  if (mode == LedMatrixEffectMode::kCatOnly) {
    currentEffect = &catOnlyEffect;
  } else {
    currentEffect = &catMessageEffect;
  }
  currentEffect->begin(matrix);

  // Re-apply whichever override is currently active so the new effect
  // starts in the right state instead of reverting to the idle animation.
  if (manualOverrideActive && serialOverrideMessage.length() > 0) {
    showOverridePayload(
      serialOverrideMessage,
      serialOverrideIcon,
      serialOverrideRed,
      serialOverrideGreen,
      serialOverrideBlue,
      "Aviso: efeito atual nao suporta overrides; override manual ignorado"
    );
  } else if (homeAssistantOverrideActive && !manualOverrideActive &&
             homeAssistantMessage.length() > 0) {
    showOverridePayload(
      homeAssistantMessage,
      homeAssistantIcon,
      homeAssistantRed,
      homeAssistantGreen,
      homeAssistantBlue,
      "Aviso: efeito atual nao suporta overrides; override HA ignorado"
    );
  }
}

inline void printSerialHelp() {
  Serial.println("Comandos:");
  Serial.println("  TEXT:mensagem livre");
  Serial.println("  ICON:nome[:mensagem]");
  Serial.print("    nomes: ");
  Serial.println(ledMatrixIconCommandList());
  Serial.println("  COLOR:r,g,b");
  Serial.println("  CLEAR");
  Serial.println("  BRIGHTNESS:n");
  Serial.println("  EFFECT:cat|playback");
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
  return ledMatrixMessageSourceLabel(resolveLedMatrixMessageSource(
    manualOverrideActive,
    homeAssistantOverrideActive
  ));
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
  Serial.print(" ultimo erro: ");
  Serial.println(client.lastErrorSummary().length() > 0 ? client.lastErrorSummary() : "<nenhum>");
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
    serialOverrideMessage = message;
    serialOverrideIcon = nullptr;
    if (showOverridePayload(
          message,
          serialOverrideIcon,
          serialOverrideRed,
          serialOverrideGreen,
          serialOverrideBlue,
          "Aviso: efeito atual (CAT ONLY) nao exibe mensagens. Use EFFECT:playback primeiro."
        )) {
      Serial.print("Override ativo: ");
      Serial.println(message);
    }
    return;
  }

  if (command.startsWith("ICON:")) {
    const LedMatrixOverridePayload payload = resolveLedMatrixOverridePayload(command);
    if (payload.icon == nullptr) {
      Serial.print("Uso: ICON:nome[:mensagem]. Nomes: ");
      Serial.println(ledMatrixIconCommandList());
      return;
    }

    manualOverrideActive = true;
    serialOverrideMessage = payload.message;
    serialOverrideIcon = payload.icon;
    if (showOverridePayload(
          serialOverrideMessage,
          serialOverrideIcon,
          serialOverrideRed,
          serialOverrideGreen,
          serialOverrideBlue,
          "Aviso: efeito atual (CAT ONLY) nao exibe icones/mensagens. Use EFFECT:playback primeiro."
        )) {
      Serial.print("Override com icone ativo: ");
      Serial.print(serialOverrideIcon->name);
      Serial.print(" -> ");
      Serial.println(serialOverrideMessage);
    }
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

  if (command.startsWith("BRIGHTNESS:")) {
    const String payload = command.substring(11);
    uint8_t brightness = runtimeBrightness;
    if (parseLedMatrixBrightnessValue(payload.c_str(), brightness)) {
      runtimeBrightness = brightness;
      matrix.setBrightness(runtimeBrightness);
      Serial.print("Brightness ajustado: ");
      Serial.println(runtimeBrightness);
    } else {
      Serial.println("Uso: BRIGHTNESS:0..255");
    }
    return;
  }

  if (command.startsWith("EFFECT:")) {
    const String payload = command.substring(7);
    LedMatrixEffectMode effectMode = currentEffectMode;
    if (parseLedMatrixEffectMode(payload.c_str(), effectMode)) {
      applyEffectMode(effectMode);
      Serial.print("Efeito ajustado: ");
      Serial.println(currentEffect->name());
    } else {
      Serial.println("Uso: EFFECT:cat|playback");
    }
    return;
  }

  if (command == "CLEAR") {
    manualOverrideActive = false;
    serialOverrideMessage = "";
    serialOverrideIcon = nullptr;
    if (homeAssistantOverrideActive) {
      if (showOverridePayload(
            homeAssistantMessage,
            homeAssistantIcon,
            homeAssistantRed,
            homeAssistantGreen,
            homeAssistantBlue,
            "Aviso: override manual limpo, mas o efeito atual nao suporta override do Home Assistant"
          )) {
        Serial.println("Override manual limpo; Home Assistant reassumiu");
      }
    } else {
      currentEffect->clearOverrideMessage(matrix);
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
    Serial.println(currentEffect->name());
    Serial.print("Effect mode: ");
    Serial.println(currentEffectMode == LedMatrixEffectMode::kCatOnly ? "CAT ONLY" : "CAT + MARQUEE");
    Serial.print("Brightness: ");
    Serial.println(runtimeBrightness);
    Serial.print("Override efetivo: ");
    printYesNo(currentEffect->hasOverrideMessage());
    Serial.print("Override manual: ");
    printYesNo(manualOverrideActive);
    Serial.print("Mensagem serial ativa: ");
    Serial.println(serialOverrideMessage.length() > 0 ? serialOverrideMessage : "<vazia>");
    Serial.print("Icone serial ativo: ");
    Serial.println(serialOverrideIcon == nullptr ? "<nenhum>" : serialOverrideIcon->name);
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
    Serial.print("Icone Home Assistant ativo: ");
    Serial.println(homeAssistantIcon == nullptr ? "<nenhum>" : homeAssistantIcon->name);
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
      showOverridePayload(
        homeAssistantMessage,
        homeAssistantIcon,
        homeAssistantRed,
        homeAssistantGreen,
        homeAssistantBlue,
        "Aviso: efeito atual nao suporta overrides; override HA ignorado"
      );
    }
  }

  bool hasMessage = false;
  String message;
  if (!homeAssistantClient.consumeStateChange(hasMessage, message)) {
    return;
  }

  homeAssistantOverrideActive = hasMessage;
  if (hasMessage) {
    const LedMatrixOverridePayload payload = resolveLedMatrixOverridePayload(message);
    homeAssistantMessage = payload.message;
    homeAssistantIcon = payload.icon;
  } else {
    homeAssistantMessage = "";
    homeAssistantIcon = nullptr;
  }

  if (manualOverrideActive) {
    Serial.println("Home Assistant atualizou, mas override manual continua prioritario");
    return;
  }

  if (hasMessage) {
    showOverridePayload(
      homeAssistantMessage,
      homeAssistantIcon,
      homeAssistantRed,
      homeAssistantGreen,
      homeAssistantBlue,
      "Aviso: efeito atual nao suporta overrides; override HA ignorado"
    );
  } else {
    currentEffect->clearOverrideMessage(matrix);
  }
}

inline void ledmatrix8x8Setup() {
  Serial.begin(115200);
  delay(1200);

  matrix.begin(PROJECT_BRIGHTNESS);
  runtimeBrightness = PROJECT_BRIGHTNESS;

  Serial.println();
  Serial.println("==== LEDMATRIX8X8 ====");
  Serial.print("Efeito atual: ");
  Serial.println(currentEffect->name());
  Serial.print("Brightness: ");
  Serial.println(runtimeBrightness);
  Serial.print("Target HA base URL: ");
  Serial.println(LEDMATRIX_HA_BASE_URL);
  printSerialHelp();

  currentEffect->begin(matrix);
  homeAssistantClient.begin();
  homeAssistantColorClient.begin();
}

inline void ledmatrix8x8Loop() {
  handleSerialInput();
  handleHomeAssistant();
  currentEffect->update(matrix);
}
