#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <WiFi.h>

#include <ArduinoJson.h>
#include <LedMatrixFirmwareCommands.h>

class HomeAssistantTextClient {
 public:
  HomeAssistantTextClient(
    const char *wifiSsid,
    const char *wifiPassword,
    bool useStaticIp,
    const char *staticIp,
    const char *gateway,
    const char *subnet,
    const char *dns1,
    const char *dns2,
    const char *baseUrl,
    const char *accessToken,
    const char *entityId,
    uint32_t pollIntervalMs,
    bool manageWifi = true
  )
      : wifiSsid_(wifiSsid == nullptr ? "" : wifiSsid),
        wifiPassword_(wifiPassword == nullptr ? "" : wifiPassword),
        useStaticIp_(useStaticIp),
        staticIp_(staticIp == nullptr ? "" : staticIp),
        gateway_(gateway == nullptr ? "" : gateway),
        subnet_(subnet == nullptr ? "" : subnet),
        dns1_(dns1 == nullptr ? "" : dns1),
        dns2_(dns2 == nullptr ? "" : dns2),
        baseUrl_(baseUrl == nullptr ? "" : baseUrl),
        accessToken_(accessToken == nullptr ? "" : accessToken),
        entityId_(entityId == nullptr ? "" : entityId),
        pollIntervalMs_(pollIntervalMs),
        manageWifi_(manageWifi) {}

  void begin() {
    if (!wifiConfigured()) {
      Serial.println("WiFi nao configurado; integracao HA desativada");
      return;
    }

    if (!apiConfigured()) {
      Serial.println("Home Assistant sem token/base URL/entity; integracao desativada");
      return;
    }

    if (manageWifi_) {
      WiFi.mode(WIFI_STA);
      WiFi.setAutoReconnect(true);
      if (useStaticIp_) {
        configureStaticIp();
      }
      connectWifi();
    }
    Serial.print("Home Assistant endpoint: ");
    Serial.println(baseUrl_ + "/api/states/" + entityId_);
  }

  void update() {
    if (!enabled()) {
      return;
    }

    if (manageWifi_) {
      maintainWifi();
    }
    if (WiFi.status() != WL_CONNECTED) {
      return;
    }

    const uint32_t now = millis();
    if (lastPollAtMs_ != 0 && (now - lastPollAtMs_) < pollIntervalMs_) {
      return;
    }

    lastPollAtMs_ = now;
    fetchMessage();
  }

  bool enabled() const {
    return wifiConfigured() && apiConfigured();
  }

  bool wifiConfiguredForDiagnostics() const {
    return wifiConfigured();
  }

  bool apiConfiguredForDiagnostics() const {
    return apiConfigured();
  }

  bool managesWifi() const {
    return manageWifi_;
  }

  bool wifiConnected() const {
    return WiFi.status() == WL_CONNECTED;
  }

  String localIpText() const {
    if (!wifiConnected()) {
      return "<desconectado>";
    }
    return WiFi.localIP().toString();
  }

  const char *wifiStatusText() const {
    switch (WiFi.status()) {
      case WL_IDLE_STATUS:
        return "IDLE";
      case WL_NO_SSID_AVAIL:
        return "SSID_INDISPONIVEL";
      case WL_SCAN_COMPLETED:
        return "SCAN_COMPLETO";
      case WL_CONNECTED:
        return "CONECTADO";
      case WL_CONNECT_FAILED:
        return "FALHA_CONEXAO";
      case WL_CONNECTION_LOST:
        return "CONEXAO_PERDIDA";
      case WL_DISCONNECTED:
        return "DESCONECTADO";
      default:
        return "DESCONHECIDO";
    }
  }

  const String &entityId() const {
    return entityId_;
  }

  int lastHttpStatus() const {
    return lastHttpStatus_;
  }

  uint32_t lastPollAtMs() const {
    return lastPollAtMs_;
  }

  bool lastStateHasMessage() const {
    return lastHasMessage_;
  }

  const String &lastState() const {
    return lastMessage_;
  }

  const String &lastErrorSummary() const {
    return lastErrorSummary_;
  }

  bool consumeStateChange(bool &hasMessage, String &message) {
    if (!hasPendingUpdate_) {
      return false;
    }

    hasPendingUpdate_ = false;
    hasMessage = pendingHasMessage_;
    message = pendingMessage_;
    return true;
  }

 private:
  bool wifiConfigured() const {
    return wifiSsid_.length() > 0;
  }

  bool apiConfigured() const {
    return (
      baseUrl_.length() > 0 &&
      accessToken_.length() > 0 &&
      entityId_.length() > 0
    );
  }

  bool parseIpAddress(const String &value, IPAddress &ipAddress) const {
    return ipAddress.fromString(value);
  }

  void configureStaticIp() {
    IPAddress localIp;
    IPAddress gatewayIp;
    IPAddress subnetMask;
    IPAddress dnsPrimary;
    IPAddress dnsSecondary;

    if (!parseIpAddress(staticIp_, localIp) ||
        !parseIpAddress(gateway_, gatewayIp) ||
        !parseIpAddress(subnet_, subnetMask) ||
        !parseIpAddress(dns1_, dnsPrimary) ||
        !parseIpAddress(dns2_, dnsSecondary)) {
      Serial.println("Falha ao parsear IP estatico; usando DHCP");
      return;
    }

    if (!WiFi.config(localIp, gatewayIp, subnetMask, dnsPrimary, dnsSecondary)) {
      Serial.println("Falha ao configurar IP estatico; usando DHCP");
      return;
    }

    Serial.print("WiFi IP estatico: ");
    Serial.println(localIp);
  }

  void connectWifi() {
    if (wifiConnectStarted_ && WiFi.status() == WL_CONNECTED) {
      return;
    }

    Serial.print("Conectando WiFi: ");
    Serial.println(wifiSsid_);
    WiFi.begin(wifiSsid_.c_str(), wifiPassword_.c_str());
    wifiConnectStarted_ = true;
    lastWifiAttemptAtMs_ = millis();
  }

  void maintainWifi() {
    if (!wifiConfigured()) {
      return;
    }

    if (WiFi.status() == WL_CONNECTED) {
      if (!wifiWasConnected_) {
        wifiWasConnected_ = true;
        Serial.print("WiFi conectado: ");
        Serial.println(WiFi.localIP());
      }
      return;
    }

    wifiWasConnected_ = false;
    const uint32_t now = millis();
    if (!wifiConnectStarted_ || (now - lastWifiAttemptAtMs_) >= 10000) {
      connectWifi();
    }
  }

  void fetchMessage() {
    HTTPClient http;
    WiFiClient client;
    const String url = baseUrl_ + "/api/states/" + entityId_;

    client.setTimeout(5000);
    http.setConnectTimeout(5000);
    http.setTimeout(5000);
    http.setReuse(false);
    http.useHTTP10(true);

    if (!http.begin(client, url)) {
      lastHttpStatus_ = 0;
      lastErrorSummary_ = "HTTP begin";
      Serial.println("Falha ao iniciar cliente HTTP do Home Assistant");
      return;
    }

    http.addHeader("Authorization", "Bearer " + accessToken_);
    http.addHeader("Accept", "application/json");
    http.addHeader("Accept-Encoding", "identity");
    http.addHeader("Connection", "close");

    const int statusCode = http.GET();
    if (statusCode != HTTP_CODE_OK) {
      Serial.print("Home Assistant HTTP status: ");
      Serial.println(statusCode);
      lastHttpStatus_ = statusCode;
      lastErrorSummary_ = formatLedMatrixHomeAssistantErrorSummary(statusCode, nullptr).c_str();
      http.end();
      return;
    }

    JsonDocument document;
    const DeserializationError error = deserializeJson(document, http.getStream());
    http.end();

    if (error) {
      Serial.print("Falha ao parsear JSON do Home Assistant: ");
      Serial.println(error.c_str());
      lastHttpStatus_ = statusCode;
      lastErrorSummary_ = formatLedMatrixHomeAssistantErrorSummary(statusCode, error.c_str()).c_str();
      return;
    }

    lastHttpStatus_ = statusCode;
    lastErrorSummary_.clear();

    const String nextMessage = document["state"] | "";
    const bool nextHasMessage = (
      nextMessage.length() > 0 &&
      nextMessage != "unknown" &&
      nextMessage != "unavailable"
    );

    if (nextHasMessage == lastHasMessage_ && nextMessage == lastMessage_) {
      return;
    }

    const bool clearedToEmpty = (!nextHasMessage && lastHasMessage_);
    const bool changedToNewMessage = (nextHasMessage && nextMessage != lastMessage_);

    lastHasMessage_ = nextHasMessage;
    lastMessage_ = nextMessage;
    pendingHasMessage_ = nextHasMessage;
    pendingMessage_ = nextHasMessage ? nextMessage : "";
    hasPendingUpdate_ = true;

    if (clearedToEmpty) {
      Serial.println("Home Assistant limpou mensagem");
    } else if (changedToNewMessage) {
      Serial.print("Home Assistant atualizou mensagem: ");
      Serial.println(nextMessage);
    } else {
      Serial.print("Home Assistant atualizou mensagem: ");
      Serial.println(nextHasMessage ? nextMessage : "<vazia>");
    }
  }

  String wifiSsid_;
  String wifiPassword_;
  bool useStaticIp_ = false;
  String staticIp_;
  String gateway_;
  String subnet_;
  String dns1_;
  String dns2_;
  String baseUrl_;
  String accessToken_;
  String entityId_;
  uint32_t pollIntervalMs_ = 5000;
  uint32_t lastWifiAttemptAtMs_ = 0;
  uint32_t lastPollAtMs_ = 0;
  int lastHttpStatus_ = 0;
  String lastErrorSummary_;
  String lastMessage_;
  String pendingMessage_;
  bool wifiConnectStarted_ = false;
  bool wifiWasConnected_ = false;
  bool lastHasMessage_ = false;
  bool pendingHasMessage_ = false;
  bool hasPendingUpdate_ = false;
  bool manageWifi_ = true;
};
