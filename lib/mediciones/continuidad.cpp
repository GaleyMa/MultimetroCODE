#include "continuidad.h"
#include "resistencia.h" // reusa el ohmetro
#include <Arduino.h>
#include <math.h>

#define UMBRAL_CONTINUIDAD 40.0f // ohms

void continuidad_setup()
{
    resistencia_setup(); // mismo front-end que resistencia
    Serial.println("[continuidad] Listo (umbral 40 ohm).");
}

bool continuidad_medir()
{
    float r = resistencia_medir(); // reusa la medicion del ohmetro
    if (isnan(r))
        return false;                // abierto -> no hay continuidad
    return (r < UMBRAL_CONTINUIDAD); // true si R < 40 ohm
}