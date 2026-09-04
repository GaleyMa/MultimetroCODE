#pragma once

// ==========================================================
//  MODO VOLTAJE (0-20 V) - divisor + buffer + MCP3208
//  Estado: VALIDADO (polaridad positiva; auto-swap pendiente)
// ==========================================================

// Inicializa la etapa de voltaje (asume que el ADC ya fue inicializado).
void voltaje_setup();

// Devuelve el voltaje de entrada en volts (calibracion de 2 puntos).
float voltaje_medir();