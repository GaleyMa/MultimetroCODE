#include "corriente.h"
#include "config.h"
#include <Arduino.h>
#include <Wire.h>

static int16_t offset = 0;

static void writeReg(uint8_t reg, uint16_t val)
{
    Wire.beginTransmission(INA226_ADDR);
    Wire.write(reg);
    Wire.write(val >> 8);
    Wire.write(val & 0xFF);
    Wire.endTransmission();
}

static int16_t readReg(uint8_t reg)
{ // devuelve con signo (two's complement)
    Wire.beginTransmission(INA226_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t)INA226_ADDR, (uint8_t)2);
    return (Wire.read() << 8) | Wire.read();
}

bool corriente_setup()
{
    Wire.begin(SDA_PIN, SCL_PIN);

    uint16_t mfg = (uint16_t)readReg(0xFE);
    Serial.printf("[corriente] Manufacturer ID: 0x%04X (esperado 0x5449)\n", mfg);
    if (mfg != 0x5449)
    {
        Serial.println("[corriente] INA226 no responde. Revisa cableado.");
        return false;
    }

    // Config: promedio 16, conversion 1.1ms, modo continuo shunt+bus
    writeReg(0x00, 0x4527);
    delay(50);

    // Offset de cero (SIN corriente en IN+/IN-)
    Serial.println("[corriente] Midiendo offset... sin carga.");
    delay(2000);
    int32_t acc = 0;
    for (int i = 0; i < 64; i++)
    {
        acc += readReg(0x01);
        delay(2);
    }
    offset = acc / 64;
    Serial.printf("[corriente] Offset: %d cuentas. Listo.\n", offset);
    return true;
}

float corriente_medir()
{
    int16_t raw = readReg(0x01) - offset;
    if (abs(raw) >= INA_SAT)
        return NAN;
    float amperes = (raw * 2.5e-6f) / RSHUNT;
    Serial.printf("raw=%d  V_shunt=%.2fmV  I=%.1fmA\n",
                  raw, raw * 2.5e-3, amperes * 1000.0f);
    return amperes * 1000.0f;
}