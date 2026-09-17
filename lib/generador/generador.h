#pragma once
#include <Arduino.h>

// Generador de funciones por PWM-DAC (Fase 2 del Multimetro).
// Sintetiza seno / triangular / cuadrada de 1 Hz a 10 kHz.
// Salida en PIN_GEN_OUT: 0-3.3V -> filtro RC -> LM358 (x3) -> 0-~10V unipolar.

void generador_setup();                                  // llamar 1 vez en setup()
void generador_aplicar(const String &w, float f, int a); // w: "sine"|"tri"|"sq", f: Hz, a: 0-100%
void generador_off();                                    // apaga la salida (duty 0)