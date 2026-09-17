#include "capacitancia.h"
#include "config.h"
#include "adc_mcp3208.h"
#include <Arduino.h>

// Umbral al 63.2% de fondo de escala -> 1 tau. raw = 0.632 * 4095
#define UMBRAL_RAW 2588
#define TIMEOUT_US 500000UL // 500ms

// Rangos de resistencia de carga
#define R_ALTA 1000000.0f // 1M  -> para nF y pF
#define R_BAJA 1000.0f    // 1k  -> para uF

// Capacitancia parasita del montaje, por rango (medida SIN capacitor)
#define C_PAR_ALTA 0.0003f // pF)con R = 1M y buffer
#define C_PAR_BAJA 0.0f    // con R=1k la parasita es despreciable

// Umbral para decidir de rango: si con R alta da mas de esto, usa R baja
#define LIMITE_RANGO 1.0f // uF

void capacitancia_setup()
{
    pinMode(PIN_CAP_DESCARGA, OUTPUT);
    digitalWrite(PIN_CAP_DESCARGA, HIGH); // arranca descargando
    pinMode(PIN_CARGA_ALTA, INPUT);       // ambas R en alta impedancia
    pinMode(PIN_CARGA_BAJA, INPUT);
    Serial.println("[capacitancia] Listo (auto-rango 1M / 1k, con buffer).");
}

// Activa la R del rango elegido; la otra queda en alta impedancia
static void seleccionar_r(bool alta)
{
    if (alta)
    {
        pinMode(PIN_CARGA_BAJA, INPUT);
        pinMode(PIN_CARGA_ALTA, OUTPUT);
        digitalWrite(PIN_CARGA_ALTA, LOW);
    }
    else
    {
        pinMode(PIN_CARGA_ALTA, INPUT);
        pinMode(PIN_CARGA_BAJA, OUTPUT);
        digitalWrite(PIN_CARGA_BAJA, LOW);
    }
}

// Mide con un rango dado. Devuelve uF crudos (sin restar parasita), o NAN.
static float medir_en_rango(bool alta)
{
    uint8_t pin = alta ? PIN_CARGA_ALTA : PIN_CARGA_BAJA;
    float R = alta ? R_ALTA : R_BAJA;

    seleccionar_r(alta);

    // 1. Descargar
    digitalWrite(pin, LOW);
    digitalWrite(PIN_CAP_DESCARGA, HIGH);
    delay(50);
    digitalWrite(PIN_CAP_DESCARGA, LOW);
    delayMicroseconds(10);

    // 2. Cargar y cronometrar hasta el umbral (63.2%)
    uint32_t t0 = micros();
    digitalWrite(pin, HIGH);

    while (adc_read(CH_CAP) < UMBRAL_RAW)
    {
        if (micros() - t0 > TIMEOUT_US)
        {
            digitalWrite(pin, LOW);
            return NAN; // no cruzo: muy grande para este rango
        }
    }
    uint32_t t = micros() - t0;
    digitalWrite(pin, LOW);

    // 3. C = t / R   (a 63.2%, ln(1/(1-0.632)) = 1)
    float C_faradios = (t * 1e-6f) / R;
    return C_faradios * 1e6f; // uF
}

float capacitancia_medir()
{
    // Intenta primero con R alta (rango nF/pF)
    float c = medir_en_rango(true);

    if (!isnan(c) && c <= LIMITE_RANGO)
    {
        float c_real = c - C_PAR_ALTA; // resta parasita del rango alto
        return (c_real < 0) ? 0.0f : c_real;
    }

    // Cap grande (o timeout con R alta): usa R baja
    c = medir_en_rango(false);
    if (isnan(c))
        return NAN; // ni con R baja: abierto o enorme

    float c_real = c - C_PAR_BAJA;
    return (c_real < 0) ? 0.0f : c_real;
}