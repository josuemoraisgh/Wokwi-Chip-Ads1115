// chips/ads1115.chip.c
// Wokwi Custom ADS1115 Chip with I²C handlers
// SPDX-License-Identifier: MIT

#include "wokwi-api.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// Estado do chip mantido globalmente
static pin_t ain[4];
static uint16_t configRegister = 0x8583;
static int selectedChannel = 0;
static uint8_t pointerRegister = 0;

// Leitura interna do conversor A/D simulada
static uint16_t readAdcValue() {
  float voltage = pin_adc_read(ain[selectedChannel]);
  return (uint16_t)((int16_t)(voltage * 32767));
}

// Inicialização do chip
__attribute__((used, visibility("default")))
void chip_init(void) {
  ain[0] = pin_init("AIN0", ANALOG);
  ain[1] = pin_init("AIN1", ANALOG);
  ain[2] = pin_init("AIN2", ANALOG);
  ain[3] = pin_init("AIN3", ANALOG);
  configRegister = 0x8583;
  selectedChannel = 0;
  pointerRegister = 0;
}

// Configuração I²C – estabelece a conexão no endereço do ADS1115
__attribute__((used, visibility("default")))
bool chip_i2c_connect(void *ctx, uint32_t i2c_index, uint32_t address) {
  return (address == 0x48); // end. padrão do ADS1115
}

// Escrita de bytes via I²C – primeiro seta ponteiro, depois configura
__attribute__((used, visibility("default")))
bool chip_i2c_write(void *ctx, uint32_t i2c_index, uint8_t data) {
  pointerRegister = (pointerRegister << 8) | data;
  if ((pointerRegister & 0xFF00) == 0x0100) {
    configRegister = pointerRegister;
    uint8_t mux = (configRegister >> 12) & 0x07;
    selectedChannel = (mux >= 4 && mux <= 7) ? (mux - 4) : 0;
  }
  return true; // ACK
}

// Leitura de bytes via I²C – retorna primeiro MSB depois LSB
__attribute__((used, visibility("default")))
uint8_t chip_i2c_read(void *ctx, uint32_t i2c_index) {
  if ((pointerRegister & 0xFF00) == 0x0000) {
    uint16_t val = readAdcValue();
    static bool msb_next = true;
    uint8_t out = msb_next ? (val >> 8) : (val & 0xFF);
    msb_next = !msb_next;
    return out;
  }
  return 0;
}

// Desconexão opcional – nada precisa ser feito
__attribute__((used, visibility("default")))
void chip_i2c_disconnect(void *ctx, uint32_t i2c_index) {
  // sem uso no momento
}