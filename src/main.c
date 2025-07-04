// chips/ads1115.chip.c
// Wokwi Custom ADS1115 Chip with I²C handlers
// SPDX-License-Identifier: MIT

#include "wokwi-api.h"
#include <stdio.h>
#include <stdint.h>

//
// Estado do chip mantido globalmente
//
static pin_t ain[4];
static uint16_t configRegister = 0x8583;
static int selectedChannel = 0;
static uint8_t pointerRegister = 0;

// Leitura interna do conversor A/D simulada
static uint16_t readAdcValue() {
  printf("ADS1115: readAdcValue() – canal atual = %d\n", selectedChannel);
  float voltage = pin_adc_read(ain[selectedChannel]);
  printf("ADS1115: tensão lida no pino = %.4f V\n", voltage);
  int16_t adc = (int16_t)(voltage * 32767);
  printf("ADS1115: convertido para valor ADC = %d\n", adc);
  return (uint16_t)adc;
}

//
// Função chamada pelo Wokwi para inicializar o chip
//
__attribute__((used, visibility("default")))
void chip_init(void) {
  printf("ADS1115: chip_init() iniciado\n");

  ain[0] = pin_init("AIN0", ANALOG);
  ain[1] = pin_init("AIN1", ANALOG);
  ain[2] = pin_init("AIN2", ANALOG);
  ain[3] = pin_init("AIN3", ANALOG);

  selectedChannel = 0;
  configRegister = 0x8583;
  pointerRegister = 0;

  printf("ADS1115: chip_init() concluído\n");
}

//
// Escrita via I2C — chamada pelo simulador quando MCU escreve dados
//
__attribute__((used, visibility("default")))
void chipI2CWrite(void *ctx, uint8_t reg, uint8_t value) {
  (void)ctx; // não usado no modelo atual

  printf("ADS1115: chipI2CWrite() - reg=0x%02X, val=0x%02X\n", reg, value);

  if (reg == 0x00) {
    pointerRegister = value;
    printf("ADS1115: ponteiro de registrador atualizado para 0x%02X\n", pointerRegister);
  } else if (reg == 0x01) {
    // Montagem do registrador de configuração em 2 etapas
    static uint8_t msb = 0;
    msb = value;
    configRegister = (msb << 8) | (configRegister & 0x00FF);
  } else if (reg == 0x02) {
    configRegister = (configRegister & 0xFF00) | value;
    int mux = (configRegister >> 12) & 0x07;

    switch (mux) {
      case 0b100: selectedChannel = 0; break;
      case 0b101: selectedChannel = 1; break;
      case 0b110: selectedChannel = 2; break;
      case 0b111: selectedChannel = 3; break;
      default:
        selectedChannel = 0;
        printf("ADS1115: MUX inválido, voltando para canal 0\n");
        break;
    }

    printf("ADS1115: novo configRegister = 0x%04X, canal = %d\n", configRegister, selectedChannel);
  } else {
    printf("ADS1115: escrita ignorada (reg=0x%02X)\n", reg);
  }
}

//
// Leitura via I2C — chamada pelo simulador quando MCU lê do registrador
//
__attribute__((used, visibility("default")))
uint8_t chipI2CRead(void *ctx, uint8_t reg) {
  (void)ctx;

  printf("ADS1115: chipI2CRead() - reg=0x%02X (ponteiro=0x%02X)\n", reg, pointerRegister);

  if (pointerRegister == 0x00) {
    // Registrador de conversão
    uint16_t val = readAdcValue();
    static bool msb_next = true;
    uint8_t result;

    if (msb_next) {
      result = (val >> 8) & 0xFF;
    } else {
      result = val & 0xFF;
    }

    msb_next = !msb_next;
    return result;
  }

  return 0x00;
}