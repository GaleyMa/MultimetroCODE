#pragma once

// ==========================================================
//  MODO CONTINUIDAD - reusa el ohmetro (resistencia)
//  Umbral: R < 40 ohm = continuidad
//  Estado: VALIDADO (sale de resistencia)
// ==========================================================

void continuidad_setup();

// true si hay continuidad (R < umbral), false si no.
bool continuidad_medir();