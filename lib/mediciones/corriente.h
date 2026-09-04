#pragma once

// ==========================================================
//  MODO CORRIENTE (0-1 A) - INA226 por I2C
//  Estado: VALIDADO
// ==========================================================

// Inicializa el INA226, verifica identidad y mide el offset de cero.
// Devuelve true si el chip respondio correctamente.
bool corriente_setup();

// Devuelve la corriente medida en miliamperes (con signo).
// Si esta fuera de rango, devuelve NAN.
float corriente_medir();