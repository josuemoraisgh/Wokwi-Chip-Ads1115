// chips/ads1115.chip.c
// Wokwi Custom ADS1115 Chip with I²C handlers
// SPDX-License-Identifier: MIT

#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
  pin_t ain[4];
  uint16_t configRegister;
  int selectedChannel;
} ads1115_state_t;

// Leitura interna do conversor A/D
static uint16_t readAdcValue(ads1115_state_t *state) {
  printf("ADS1115: readAdcValue() – canal atual = %d\n", state->selectedChannel);
  float voltage = pin_adc_read(state->ain[state->selectedChannel]);
  printf("ADS1115: pin_adc_read retornou tensão = %f V\n", voltage);
  int16_t adc = (int16_t)(voltage * 32767);
  printf("ADS1115: convertido para valor ADC = %d\n", adc);
  return (uint16_t)adc;
}

// Handler de leitura I²C com assinatura correta
__attribute__((used, visibility("default")))
uint8_t chipI2CRead(void *ctx, uint8_t reg) {
  ads1115_state_t *state = (ads1115_state_t *)ctx;

  printf("ADS1115: chipI2CRead() chamado – reg=0x%02X\n", reg);

  if (reg == 0x00) {
    uint16_t val = readAdcValue(state);
    printf("ADS1115: retornando MSB=0x%02X\n", (val >> 8) & 0xFF);
    return (val >> 8) & 0xFF;  // primeira parte da leitura
  }

  // Simples retorno de 0 para outros registradores
  printf("ADS1115: registrador 0x%02X não suportado na leitura única\n", reg);
  return 0x00;
}

// Handler de escrita I²C com assinatura correta
__attribute__((used, visibility("default")))
void chipI2CWrite(void *ctx, uint8_t reg, uint8_t value) {
  ads1115_state_t *state = (ads1115_state_t *)ctx;

  printf("ADS1115: chipI2CWrite() chamado – reg=0x%02X, val=0x%02X\n", reg, value);

  // Aqui estamos montando o configRegister em dois passos (MSB e LSB)
  static uint8_t config_msb = 0;

  if (reg == 0x01) {
    config_msb = value;
  } else if (reg == 0x02) {
    state->configRegister = ((uint16_t)config_msb << 8) | value;
    printf("ADS1115: configRegister = 0x%04X\n", state->configRegister);

    int mux = (state->configRegister >> 12) & 0x07;
    switch (mux) {
      case 0b100: state->selectedChannel = 0; break;
      case 0b101: state->selectedChannel = 1; break;
      case 0b110: state->selectedChannel = 2; break;
      case 0b111: state->selectedChannel = 3; break;
      default:
        state->selectedChannel = 0;
        printf("ADS1115: MUX inválido, revertendo canal 0\n");
        break;
    }

    printf("ADS1115: canal selecionado = %d\n", state->selectedChannel);
  }
}

// Inicialização do chip
__attribute__((used, visibility("default")))
void chip_init(void *ctx) {
  ads1115_state_t *state = (ads1115_state_t *)ctx;

  printf("ADS1115: chip_init() iniciado\n");

  state->ain[0] = pin_init("AIN0", ANALOG);
  state->ain[1] = pin_init("AIN1", ANALOG);
  state->ain[2] = pin_init("AIN2", ANALOG);
  state->ain[3] = pin_init("AIN3", ANALOG);

  state->configRegister = 0x8583;
  state->selectedChannel = 0;

  printf("ADS1115: chip_init() concluído\n");
}