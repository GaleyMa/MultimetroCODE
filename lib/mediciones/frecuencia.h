#pragma once

// ==========================================================
//  MODO FRECUENCIA (0-10 kHz) - PCNT del ESP32-S3
//  Estado: EN VALIDACION
// ==========================================================

void frecuencia_setup();

// Devuelve la frecuencia medida en Hz.
float frecuencia_medir();