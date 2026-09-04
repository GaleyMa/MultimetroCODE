#pragma once
#pragma once

// ==========================================================
//  MODO RESISTENCIA (0-100 kOhm) - ratiometrico con auto-rango
//  Selecciona Rref (1k/10k/100k) via 3 GPIOs (sin reles por ahora).
//  Estado: EN VALIDACION
// ==========================================================

void resistencia_setup();

// Devuelve la resistencia en ohms (elige rango automaticamente).
// NAN si esta abierto; 0 si hay continuidad/corto.
float resistencia_medir();