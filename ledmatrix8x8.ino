/*
 * LED Matrix 8x8 — Contador com ESP32-S3
 *
 * Exibe digitos 0-9 em uma matriz NeoPixel 8x8 com cores do espectro HSV.
 * Inclui teste inicial e transicao com fade entre digitos.
 *
 * Hardware: ESP32-S3-DevKitC-1 + Matriz WS2812B 8x8
 * Pino de dados: GPIO38 (NAO usar GPIO48, que eh o LED RGB onboard)
 *
 * Versao Arduino IDE — equivalente a src/main.cpp (PlatformIO)
 */

#include <Adafruit_NeoPixel.h>

// === CONFIGURACAO ===
// ATENCAO: GPIO48 eh o LED RGB onboard do ESP32-S3-DevKitC-1.
// Para matriz externa, use GPIO livre como GPIO38, GPIO4, GPIO5, etc.
#define PIN_LED            38
#define LARGURA            8
#define ALTURA             8
#define NUM_LEDS           (LARGURA * ALTURA)
#define BRILHO             40
#define TEMPO_DIGITO_MS    700
#define FADE_PASSOS        10
#define FADE_DELAY_MS      25

// Ajuste conforme a sua matriz
#define MATRIZ_ZIGZAG      1   // 1 = serpentina (linhas impares invertidas)
#define ORIGEM_EMBAIXO     0   // 1 = pixel 0 no canto inferior esquerdo

Adafruit_NeoPixel matrix(NUM_LEDS, PIN_LED, NEO_GRB + NEO_KHZ800);

// Bitmap 5x7 para cada digito (MSB = coluna esquerda)
const uint8_t DIGITOS_5X7[10][7] = {
  {0b01110, 0b10001, 0b10011, 0b10101, 0b11001, 0b10001, 0b01110}, // 0
  {0b00100, 0b01100, 0b00100, 0b00100, 0b00100, 0b00100, 0b01110}, // 1
  {0b01110, 0b10001, 0b00001, 0b00010, 0b00100, 0b01000, 0b11111}, // 2
  {0b11110, 0b00001, 0b00001, 0b01110, 0b00001, 0b00001, 0b11110}, // 3
  {0b00010, 0b00110, 0b01010, 0b10010, 0b11111, 0b00010, 0b00010}, // 4
  {0b11111, 0b10000, 0b10000, 0b11110, 0b00001, 0b00001, 0b11110}, // 5
  {0b01110, 0b10000, 0b10000, 0b11110, 0b10001, 0b10001, 0b01110}, // 6
  {0b11111, 0b00001, 0b00010, 0b00100, 0b01000, 0b10000, 0b10000}, // 7
  {0b01110, 0b10001, 0b10001, 0b01110, 0b10001, 0b10001, 0b01110}, // 8
  {0b01110, 0b10001, 0b10001, 0b01111, 0b00001, 0b00001, 0b01110}  // 9
};

uint16_t xyParaIndice(uint8_t x, uint8_t y) {
#if ORIGEM_EMBAIXO
  y = (ALTURA - 1) - y;
#endif

#if MATRIZ_ZIGZAG
  if (y & 0x01) {
    x = (LARGURA - 1) - x;
  }
#endif

  return (uint16_t)y * LARGURA + x;
}

void setXY(uint8_t x, uint8_t y, uint32_t cor) {
  if (x >= LARGURA || y >= ALTURA) {
    return;
  }
  matrix.setPixelColor(xyParaIndice(x, y), cor);
}

uint32_t corDoNumero(uint8_t n) {
  uint16_t hue = (uint16_t)n * 6554;
  return matrix.gamma32(matrix.ColorHSV(hue, 255, 255));
}

void desenharDigito(uint8_t numero, uint32_t cor, bool mostrar = true) {
  if (numero > 9) {
    return;
  }

  matrix.clear();

  const uint8_t offsetX = 1;
  const uint8_t offsetY = 1;

  for (uint8_t y = 0; y < 7; y++) {
    uint8_t bits = DIGITOS_5X7[numero][y];
    for (uint8_t x = 0; x < 5; x++) {
      if (bits & (1 << (4 - x))) {
        setXY(x + offsetX, y + offsetY, cor);
      }
    }
  }

  if (mostrar) {
    matrix.show();
  }
}

void fadeOut() {
  for (int passo = FADE_PASSOS; passo >= 0; passo--) {
    uint8_t fator = (uint8_t)((passo * 255) / FADE_PASSOS);
    matrix.setBrightness(((uint16_t)BRILHO * fator) >> 8);
    matrix.show();
    delay(FADE_DELAY_MS);
  }
  matrix.setBrightness(BRILHO);
}

void fadeIn(uint8_t numero, uint32_t cor) {
  desenharDigito(numero, cor, false);
  for (int passo = 0; passo <= FADE_PASSOS; passo++) {
    uint8_t fator = (uint8_t)((passo * 255) / FADE_PASSOS);
    matrix.setBrightness(((uint16_t)BRILHO * fator) >> 8);
    matrix.show();
    delay(FADE_DELAY_MS);
  }
  matrix.setBrightness(BRILHO);
}

void testeInicial() {
  const uint32_t cores[] = {
    matrix.Color(255, 0, 0),
    matrix.Color(0, 255, 0),
    matrix.Color(0, 0, 255),
  };

  for (uint8_t c = 0; c < 3; c++) {
    matrix.fill(cores[c]);
    matrix.show();
    delay(350);
  }

  matrix.clear();
  matrix.show();
  delay(200);

  for (uint16_t i = 0; i < NUM_LEDS; i++) {
    matrix.clear();
    matrix.setPixelColor(i, matrix.Color(255, 255, 255));
    matrix.show();
    delay(30);
  }

  matrix.clear();
  matrix.show();
  delay(200);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println();
  Serial.println("=== LED Matrix 8x8 - ESP32-S3 ===");
  Serial.print("Pino de dados: GPIO");
  Serial.println(PIN_LED);
  Serial.print("LEDs: ");
  Serial.println(NUM_LEDS);

  matrix.begin();
  matrix.setBrightness(BRILHO);
  matrix.clear();
  matrix.show();

  Serial.println("Iniciando teste da matriz...");
  testeInicial();
  Serial.println("Teste finalizado. Iniciando contador 0-9.");
}

void loop() {
  for (uint8_t n = 0; n <= 9; n++) {
    uint32_t cor = corDoNumero(n);

    fadeIn(n, cor);

    Serial.print("Digito: ");
    Serial.println(n);

    delay(TEMPO_DIGITO_MS);

    fadeOut();
  }
}
