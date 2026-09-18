#include "generador.h"
#include "config.h"
#include <math.h>

// ============================================================================
//  Generador de funciones por PWM-DAC — DDS por software (acumulador de fase).
//  Escrito para Arduino-ESP32 core 2.x (mismo que usa tu modulo de frecuencia).
//  El AD9833 no se usa: las tres ondas salen del ESP32.
// ============================================================================

#define GEN_PWM_BITS 6         // antes: 8
#define GEN_PWM_CARRIER 500000 // antes: 150000 — ahora ~3.3x más lejos del corte RC
#define GEN_SAMPLE_RATE 125000 // Hz del ISR. 1e6/GEN_SAMPLE_RATE debe ser ENTERO (aqui 10)
#define GEN_LUT_SIZE 256

// Tablas 0..255. Minimo en 0 -> onda unipolar (valle 0, pico en el maximo),
// asi el escalado de amplitud deja el valle en 0 y solo mueve el pico.
static uint8_t lutSine[GEN_LUT_SIZE];
static uint8_t lutTri[GEN_LUT_SIZE];
static uint8_t lutSqr[GEN_LUT_SIZE];
static uint32_t g_lfsr = 0xACE1u; // semilla del generador pseudoaleatorio

// Estado compartido ISR <-> web (volatile). Escrituras de 32 bits son atomicas.
volatile uint32_t g_phase = 0;
volatile uint32_t g_phaseInc = 0;
volatile uint8_t g_amp = 255;
volatile const uint8_t *g_lut = lutSine;
volatile bool g_on = false;

static hw_timer_t *g_timer = nullptr;

static inline uint8_t gen_dither_bit()
{
    // LFSR de 16 bits, rápido y determinista, suficiente para dither
    g_lfsr ^= g_lfsr << 7;
    g_lfsr ^= g_lfsr >> 9;
    g_lfsr ^= g_lfsr << 8;
    return (uint8_t)(g_lfsr & 0x0F); // ruido de 0 a 15
}

void IRAM_ATTR gen_isr()
{
    if (!g_on)
    {
        ledcWrite(GEN_LEDC_CH, 0);
        return;
    }
    g_phase += g_phaseInc;
    uint8_t idx = g_phase >> 24;
    uint16_t raw = ((uint16_t)g_lut[idx] * g_amp);              // producto sin truncar aún (16 bits)
    uint16_t dithered = raw + gen_dither_bit();                 // suma el ruido antes de truncar
    uint16_t duty = (((uint16_t)g_lut[idx] * g_amp) >> 8) >> 2; // >>2 extra: escala de 8 bits a 6 bits
    if (duty > 255)
        duty = 255; // clamp por si el dither se pasa
    ledcWrite(GEN_LEDC_CH, duty);
}
static void gen_build_luts()
{
    for (int i = 0; i < GEN_LUT_SIZE; i++)
    {
        lutSine[i] = (uint8_t)lroundf(127.5f * (1.0f + sinf(2.0f * PI * i / GEN_LUT_SIZE)));
        lutTri[i] = (i < GEN_LUT_SIZE / 2)
                        ? (uint8_t)(i * 255 / (GEN_LUT_SIZE / 2))
                        : (uint8_t)((GEN_LUT_SIZE - i) * 255 / (GEN_LUT_SIZE / 2));
        lutSqr[i] = (i < GEN_LUT_SIZE / 2) ? 0 : 255;
    }
}

void generador_setup()
{
    gen_build_luts();

    // --- PWM portadora (API core 2.x). Canal != 0 (ch0 lo usa el test-gen de frecuencia) ---
    ledcSetup(GEN_LEDC_CH, GEN_PWM_CARRIER, GEN_PWM_BITS);
    ledcAttachPin(PIN_GEN_OUT, GEN_LEDC_CH);
    ledcWrite(GEN_LEDC_CH, 0);

    // --- Timer de muestreo: prescaler 80 sobre 80MHz -> 1 MHz de tick ---
    g_timer = timerBegin(GEN_TIMER_NUM, 80, true);
    timerAttachInterrupt(g_timer, &gen_isr, true);
    timerAlarmWrite(g_timer, 1000000 / GEN_SAMPLE_RATE, true); // cada N ticks -> GEN_SAMPLE_RATE
    timerAlarmEnable(g_timer);

    Serial.printf("[generador] Listo en GPIO%d (LEDC ch%d, timer %d).\n",
                  PIN_GEN_OUT, GEN_LEDC_CH, GEN_TIMER_NUM);
}

static inline uint32_t gen_freq_to_inc(float f)
{
    return (uint32_t)((f * 4294967296.0f) / GEN_SAMPLE_RATE); // 2^32
}

void generador_aplicar(const String &w, float f, int a)
{
    if (f < 0)
        f = 0;
    if (f > 10000)
        f = 10000; // spec: hasta 10 kHz
    if (a < 0)
        a = 0;
    if (a > 100)
        a = 100; // amplitud en %

    if (w == "tri")
        g_lut = lutTri;
    else if (w == "sq")
        g_lut = lutSqr;
    else
        g_lut = lutSine;

    g_amp = (uint8_t)(a * 255 / 100);
    g_phaseInc = gen_freq_to_inc(f);
    g_on = (f > 0 && a > 0);
}

void generador_off()
{
    g_on = false;
}