#include <Arduino.h>
#include "config.h"
#include "adc_mcp3208.h"
#include "corriente.h"
#include "voltaje.h"
#include "resistencia.h"
#include "frecuencia.h"
#include "capacitancia.h"
#include "continuidad.h"

#define MODO_CORRIENTE 1
#define MODO_VOLTAJE 2
#define MODO_RESISTENCIA 3
#define MODO_FRECUENCIA 5
#define MODO_CAPACITANCIA 4
#define MODO_CONTINUIDAD 6
#define MODO_PRUEBA 99

#define PIN_RELE 40

int modo_activo = MODO_PRUEBA;

void setup()
{
  Serial.begin(115200);
  delay(1500);
  Serial.println("\n=== MULTIMETRO INICIADO ===");

  adc_init(); // el ADC lo usan varios modos: se inicializa siempre

  switch (modo_activo)
  {
  case MODO_CORRIENTE:
    if (!corriente_setup())
    {
      Serial.println("Fallo init corriente. Deteniendo.");
      while (true)
        delay(1000);
    }
    break;
  case MODO_VOLTAJE:
    voltaje_setup();
    break;
  case MODO_RESISTENCIA:
    resistencia_setup();
    break;
  case MODO_FRECUENCIA:
    frecuencia_setup();
    break;
  case MODO_CAPACITANCIA:
    capacitancia_setup();
    break;
  case MODO_CONTINUIDAD:
    continuidad_setup();
    break;
  case MODO_PRUEBA:
    Serial.println("[modo prueba] Rele ON/OFF cada 2s");
    pinMode(PIN_RELE, OUTPUT);
  }
}

void loop()
{
  switch (modo_activo)
  {
  case MODO_CORRIENTE:
  {
    float mA = corriente_medir();
    if (isnan(mA))
      Serial.println("I = FUERA DE RANGO");
    else
      Serial.printf("I = %8.2f mA\n", mA);
    break;
  }
  case MODO_VOLTAJE:
  {
    float v = voltaje_medir();
    Serial.printf("V = %.2f V\n", v);
    break;
  }
  case MODO_RESISTENCIA:
  {
    float r = resistencia_medir();
    if (isnan(r))
      Serial.println("R = ABIERTO");
    else if (r < 1.0)
      Serial.println("R = CONTINUIDAD (~0)");
    else
      Serial.printf("R = %.0f\n", r);
    break;
  }
  case MODO_FRECUENCIA:
  {
    float f = frecuencia_medir();
    Serial.printf("F = %.0f Hz\n", f);
    break;
  }
  case MODO_CAPACITANCIA:
  {
    float c = capacitancia_medir();
    if (isnan(c))
      Serial.println("C = ABIERTO o MUY GRANDE");
    else
      Serial.printf("C = %.2f uF\n", c);
    break;
  }
  case MODO_CONTINUIDAD:
  {
    bool hay_continuidad = continuidad_medir();
    if (hay_continuidad)
      Serial.println("CONTINUIDAD DETECTADA");
    else
      Serial.println("NO HAY CONTINUIDAD");
    break;
  }
  case MODO_PRUEBA:

    digitalWrite(PIN_RELE, LOW);
    Serial.println("[modo prueba] Rele OFF");
    }
  // elay(300);
}