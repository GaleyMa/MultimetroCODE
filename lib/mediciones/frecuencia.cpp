#include "frecuencia.h"
#include "config.h"
#include <Arduino.h>
#include "driver/pcnt.h" // legacy (el que SÍ tienes)

#define PCNT_UNIDAD PCNT_UNIT_0

void frecuencia_setup()
{
    ledcSetup(0, 10000, 10);        // 10 bits (funciona en todo el rango)
    ledcAttachPin(PIN_FREC_GEN, 0); // GPIO17 genera
    ledcWrite(0, 512);

    pcnt_config_t cfg = {};
    cfg.pulse_gpio_num = PIN_FREC_IN;
    cfg.ctrl_gpio_num = PCNT_PIN_NOT_USED;
    cfg.channel = PCNT_CHANNEL_0;
    cfg.unit = PCNT_UNIDAD;
    cfg.pos_mode = PCNT_COUNT_INC;
    cfg.neg_mode = PCNT_COUNT_DIS;
    cfg.counter_h_lim = 32767;
    cfg.counter_l_lim = -32768;
    pcnt_unit_config(&cfg);

    pcnt_set_filter_value(PCNT_UNIDAD, 100);
    pcnt_filter_enable(PCNT_UNIDAD);

    pcnt_counter_pause(PCNT_UNIDAD);
    pcnt_counter_clear(PCNT_UNIDAD);
    pcnt_counter_resume(PCNT_UNIDAD);

    Serial.println("[frecuencia] PCNT listo. Puentea GPIO17 -> GPIO18.");
}

float frecuencia_medir()
{
    int16_t count = 0;
    pcnt_counter_clear(PCNT_UNIDAD);
    delay(1000);
    pcnt_get_counter_value(PCNT_UNIDAD, &count); // <-- get EN MEDIO
    return (float)count;
}