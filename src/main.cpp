/*
 * LED Matrix 8x8 — Contador com ESP32-S3
 *
 * Exibe digitos 0-9 em uma matriz NeoPixel 8x8 com cores do espectro HSV.
 * Inclui diagnostico de mapeamento e transicao com fade.
 *
 * Hardware: ESP32-S3-DevKitC-1 + Matriz WS2812B 8x8
 * Pino de dados: GPIO38
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

// === CONFIGURACAO ===
#define PIN_LED            38
#define LARGURA            8
#define ALTURA             8
#define NUM_LEDS           (LARGURA * ALTURA)
#define BRILHO             255
#define TEMPO_DIGITO_MS    1500
#define FADE_PASSOS        10
#define FADE_DELAY_MS      25

// Mapeamento confirmado: SEM zigzag, origem CIMA
#define MATRIZ_ZIGZAG      0
#define ORIGEM_EMBAIXO     0

Adafruit_NeoPixel matrix(NUM_LEDS, PIN_LED, NEO_GRB + NEO_KHZ800);

// font8x8_basic — fonte publica IBM VGA (Daniel Hepper / Marcel Sondaar)
// Formato: 8 bytes por glifo, 1 byte por linha, LSB = coluna esquerda
const uint8_t FONT8X8_DIGITS[10][8] = {
  {0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00}, // 0
  {0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00}, // 1
  {0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00}, // 2
  {0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00}, // 3
  {0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00}, // 4
  {0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00}, // 5
  {0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00}, // 6
  {0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00}, // 7
  {0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00}, // 8
  {0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00}  // 9
};

// === Mapeamento de pixel ===
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
  if (x >= LARGURA || y >= ALTURA) return;
  matrix.setPixelColor(xyParaIndice(x, y), cor);
}

// Cor HSV unica por digito
uint32_t corDoNumero(uint8_t n) {
  uint16_t hue = (uint16_t)n * 6554;
  return matrix.gamma32(matrix.ColorHSV(hue, 255, 255));
}

// Escala uma cor RGB por um fator (0-255)
uint32_t escalarCor(uint32_t cor, uint8_t fator) {
  uint8_t r = (uint8_t)(cor >> 16);
  uint8_t g = (uint8_t)(cor >> 8);
  uint8_t b = (uint8_t)(cor);
  r = ((uint16_t)r * fator) >> 8;
  g = ((uint16_t)g * fator) >> 8;
  b = ((uint16_t)b * fator) >> 8;
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

// === Renderizacao ===
// font8x8 usa LSB = coluna esquerda, 8 linhas por glifo
void desenharDigito(uint8_t numero, uint32_t cor) {
  if (numero > 9) return;

  matrix.clear();

  for (uint8_t y = 0; y < 8; y++) {
    uint8_t bits = FONT8X8_DIGITS[numero][y];
    for (uint8_t x = 0; x < 8; x++) {
      if (bits & (1 << x)) {      // LSB = coluna 0 (esquerda)
        setXY(x, y, cor);
      }
    }
  }

  matrix.show();
}

// === Efeitos de fade ===
void fadeOut(uint8_t numero, uint32_t cor) {
  for (int passo = FADE_PASSOS; passo >= 0; passo--) {
    uint8_t fator = (uint8_t)((passo * 255) / FADE_PASSOS);
    desenharDigito(numero, escalarCor(cor, fator));
    delay(FADE_DELAY_MS);
  }
}

void fadeIn(uint8_t numero, uint32_t cor) {
  for (int passo = 0; passo <= FADE_PASSOS; passo++) {
    uint8_t fator = (uint8_t)((passo * 255) / FADE_PASSOS);
    desenharDigito(numero, escalarCor(cor, fator));
    delay(FADE_DELAY_MS);
  }
}



// === Setup ===
void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n\n===== LED MATRIX 8x8 =====");
  Serial.print("GPIO: ");
  Serial.println(PIN_LED);
  Serial.print("LEDs: ");
  Serial.println(NUM_LEDS);
  Serial.print("Brilho: ");
  Serial.println(BRILHO);
  Serial.println("===========================\n");
  Serial.flush();

  matrix.begin();
  matrix.setBrightness(BRILHO);
  matrix.clear();
  matrix.show();

  Serial.println("Iniciando contador 0-9...\n");
}

// === Loop principal ===
void loop() {
  for (uint8_t n = 0; n <= 9; n++) {
    uint32_t cor = corDoNumero(n);

    fadeIn(n, cor);

    Serial.print("Digito: ");
    Serial.println(n);
    Serial.flush();

    delay(TEMPO_DIGITO_MS);

    fadeOut(n, cor);
  }
}
