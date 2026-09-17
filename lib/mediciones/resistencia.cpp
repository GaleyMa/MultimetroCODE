#include "resistencia.h"
#include "config.h"
#include "adc_mcp3208.h"
#include <Arduino.h>

#define N_RREF 7
#define ADC_FS 4095.0f
#define SWITCH_RESISTANCE 60.0f // Ron del 4051 a 3.3V (CALIBRAR, ver abajo)

// Rref por canal del mux (Y0..Y6)
static const float rRef[N_RREF] = {1000, 330, 100, 3300, 10000, 33000, 100000};
//                                  Y0    Y1   Y2   Y3    Y4     Y5     Y6
static void seleccionar_canal(int n)
{
    digitalWrite(PIN_MUX_S0, n & 1);
    digitalWrite(PIN_MUX_S1, (n >> 1) & 1);
    digitalWrite(PIN_MUX_S2, (n >> 2) & 1);
}

void resistencia_setup()
{
    pinMode(PIN_MUX_S0, OUTPUT);
    pinMode(PIN_MUX_S1, OUTPUT);
    pinMode(PIN_MUX_S2, OUTPUT);
    seleccionar_canal(N_RREF - 1); // estaciona en 100k (mínima corriente)
    Serial.println("[resistencia] MUX auto-rango listo (7 Rref).");
}

float resistencia_medir()
{
    int mejorIdx = 0, mejorDelta = 100000;
    uint16_t mejorRaw = 0;

    for (int i = 0; i < N_RREF; i++)
    {
        seleccionar_canal(i);
        delay(5);
        uint16_t raw = adc_read_avg(CH_RES_JUNCTION, 32); // CH2 = vía buffer
        int delta = abs((int)raw - 2048);
        if (delta < mejorDelta)
        {
            mejorDelta = delta;
            mejorRaw = raw;
            mejorIdx = i;
        }
    }
    seleccionar_canal(N_RREF - 1);

    if (mejorRaw >= ADC_FS - 10)
        return NAN;
    if (mejorRaw <= 10)
        return 0.0f;

    float rrefEff = rRef[mejorIdx] + SWITCH_RESISTANCE;
    float rx = rrefEff * (float)mejorRaw / (ADC_FS - (float)mejorRaw);
    Serial.printf("[R] Rref=%.0f raw=%d Rx=%.0f\n", rRef[mejorIdx], mejorRaw, rx);
    return rx;
}