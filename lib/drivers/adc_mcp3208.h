#pragma once
#include <Arduino.h>

// ==========================================================
//  DRIVER MCP3208 - ADC externo 12 bits, 8 canales, SPI
//  Compartido por los modos: voltaje, resistencia, capacitancia.
//  Solo sabe leer canales; no sabe nada de magnitudes.
// ==========================================================

// Inicializa el bus SPI del ADC. Llamar una sola vez.
void adc_init();

// Lee un canal (0-7) una vez. Devuelve el valor crudo (0-4095).
uint16_t adc_read(uint8_t canal);

// Lee un canal promediando n muestras (baja ruido). Devuelve crudo.
uint16_t adc_read_avg(uint8_t canal, int n = 16);