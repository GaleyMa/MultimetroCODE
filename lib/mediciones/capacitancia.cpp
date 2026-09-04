#include "capacitancia.h"
#include "config.h"
#include "adc_mcp3208.h"
#include <Arduino.h>

// Umbral al 63.2% de fondo de escala -> a 1 tau. raw = 0.632 * 4095
#define UMBRAL_RAW 2588
#define TIMEOUT_US 500000UL // 500ms: si no cruza, cap muy grande o abierto

void capacitancia_setup()
{
    pinMode(PIN_CAP_CARGA, OUTPUT);
    pinMode(PIN_CAP_DESCARGA, OUTPUT);
    digitalWrite(PIN_CAP_CARGA, LOW);
    digitalWrite(PIN_CAP_DESCARGA, HIGH); // arranca descargando
    Serial.println("[capacitancia] Listo (R=1k, deteccion por ADC).");
}

float capacitancia_medir()
{
    // 1. Descargar
    digitalWrite(PIN_CAP_CARGA, LOW);
    digitalWrite(PIN_CAP_DESCARGA, HIGH);
    delay(50);
    digitalWrite(PIN_CAP_DESCARGA, LOW);
    delayMicroseconds(10);

    // 2. Cargar y cronometrar hasta el umbral (63.2%)
    uint32_t t0 = micros();
    digitalWrite(PIN_CAP_CARGA, HIGH);

    while (adc_read(CH_CAP) < UMBRAL_RAW)
    {
        if (micros() - t0 > TIMEOUT_US)
        {
            digitalWrite(PIN_CAP_CARGA, LOW);
            return NAN;
        }
    }
    uint32_t t = micros() - t0;
    digitalWrite(PIN_CAP_CARGA, LOW);

    // 3. C = t / R  (a 63.2%, ln(1/(1-0.632)) = 1)
    float C_faradios = (t * 1e-6f) / R_CARGA;
    return C_faradios * 1e6f; // µF
}