#pragma once

/*
 * LED Matrix 8x8 - app selecionando um efeito importavel.
 *
 * Para trocar a animacao exibida, substitua a instancia de currentEffect por
 * outro efeito que implemente LedMatrixEffect.
 */

#include <Arduino.h>

#include <CatAnimation.h>
#include <LedMatrixCore.h>

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

static CatAnimationEffect currentEffect(PROJECT_CAT_FRAME_MS);

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

  currentEffect.begin(matrix);
}

inline void ledmatrix8x8Loop() {
  currentEffect.update(matrix);
}
