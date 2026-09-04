#include "voltaje.h"
#include "config.h"
#include "adc_mcp3208.h"
#include <Arduino.h>

void voltaje_setup()
{
    // El ADC se inicializa una sola vez desde main (adc_init).
    // Aqui no hay nada extra que preparar por ahora.
    Serial.println("[voltaje] Listo (calibracion de 2 puntos).");
}

float voltaje_medir()
{
    uint16_t raw = adc_read_avg(CH_VOLTAJE);
    // Interpolacion lineal entre los dos puntos de calibracion
    float v = V_CAL_BAJO +
              (raw - RAW_CAL_BAJO) * (V_CAL_ALTO - V_CAL_BAJO) /
                  (RAW_CAL_ALTO - RAW_CAL_BAJO);
    if (v < 0)
        v = 0; // recorta ruido cerca de 0 (ver nota de polaridad en la doc)
    return v;
}