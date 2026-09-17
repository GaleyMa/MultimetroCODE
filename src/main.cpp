#include <Arduino.h>
#include "config.h"
#include "adc_mcp3208.h"
#include "corriente.h"
#include "voltaje.h"
#include "resistencia.h"
#include "frecuencia.h"
#include "capacitancia.h"
#include "continuidad.h"
#include "servidor.h"
#include "generador.h"

#define MODO_CORRIENTE 1
#define MODO_VOLTAJE 2
#define MODO_RESISTENCIA 3
#define MODO_CAPACITANCIA 4
#define MODO_FRECUENCIA 5
#define MODO_CONTINUIDAD 6
#define OSC_MUESTRAS 200
int modo_activo = MODO_VOLTAJE; // modo inicial al arrancar

// ---------------- Control de reles ----------------
const uint8_t RELE[4] = {REL_CORRIENTE, REL_VOLTAJE, REL_RESIST, REL_CAPACIT};

void reles_setup()
{
  for (int i = 0; i < 4; i++)
  {
    digitalWrite(RELE[i], RELE_OFF); // estado seguro ANTES de ser salida
    pinMode(RELE[i], OUTPUT);
  }
}

void seleccionar_modo(int modo)
{
  for (int i = 0; i < 4; i++)
    digitalWrite(RELE[i], RELE_OFF); // todos OFF primero

  switch (modo)
  {
  case MODO_CORRIENTE:
    digitalWrite(REL_CORRIENTE, RELE_ON);
    break;
  case MODO_VOLTAJE:
    digitalWrite(REL_VOLTAJE, RELE_ON);
    break;
  case MODO_RESISTENCIA:
    digitalWrite(REL_RESIST, RELE_ON);
    break;
  case MODO_CAPACITANCIA:
    digitalWrite(REL_CAPACIT, RELE_ON);
    break;
  case MODO_FRECUENCIA: /* sin rele: comparador fijo */
    break;
  case MODO_CONTINUIDAD:
    digitalWrite(REL_RESIST, RELE_ON);
    break;
  }
}

// ---------------- Inicializa el front-end del modo ----------------
static void init_modo(int modo)
{
  switch (modo)
  {
  case MODO_CORRIENTE:
    if (!corriente_setup())
      Serial.println("[main] Fallo init corriente.");
    break;
  case MODO_VOLTAJE:
    voltaje_setup();
    break;
  case MODO_RESISTENCIA:
    resistencia_setup();
    break;
  case MODO_CAPACITANCIA:
    capacitancia_setup();
    break;
  case MODO_FRECUENCIA:
    frecuencia_setup();
    break;
  case MODO_CONTINUIDAD:
    continuidad_setup();
    break;
  }
}

// ---------------- Cambio de modo (lo llama el servidor) ----------------
void cambiar_modo(int nuevo)
{
  if (nuevo < 1 || nuevo > 6)
    return;
  modo_activo = nuevo;
  seleccionar_modo(nuevo); // conmuta el rele
  delay(50);               // deja asentar el contacto
  init_modo(nuevo);        // inicializa el front-end
  Serial.printf("[main] Modo cambiado a %d\n", nuevo);
}

// Captura un bloque de muestras del canal de voltaje.
// Devuelve JSON: {"dt":50,"d":[v1,v2,...]}  (dt en microsegundos)
String capturar_osciloscopio()
{
  static uint16_t buf[OSC_MUESTRAS];

  uint32_t t0 = micros();
  for (int i = 0; i < OSC_MUESTRAS; i++)
  {
    buf[i] = adc_read(CH_VOLTAJE);
  }
  uint32_t dt_total = micros() - t0;
  uint32_t dt = dt_total / OSC_MUESTRAS; // microsegundos por muestra

  // arma el JSON con los voltajes reconstruidos
  String json = "{\"dt\":" + String(dt) + ",\"d\":[";
  for (int i = 0; i < OSC_MUESTRAS; i++)
  {
    float v = V_CAL_BAJO + (buf[i] - RAW_CAL_BAJO) *
                               (V_CAL_ALTO - V_CAL_BAJO) / (RAW_CAL_ALTO - RAW_CAL_BAJO);
    if (v < 0)
      v = 0;
    json += String(v, 2);
    if (i < OSC_MUESTRAS - 1)
      json += ",";
  }
  json += "]}";
  return json;
}

// ---------------- Medicion formateada (la pide el servidor) ----------------
String leer_medicion()
{
  char buf[32];

  switch (modo_activo)
  {
  case MODO_CORRIENTE:
  {
    float mA = corriente_medir();
    if (isnan(mA))
      return "fuera de rango";
    snprintf(buf, sizeof(buf), "%.2f mA", mA);
    return String(buf);
  }
  case MODO_VOLTAJE:
  {
    snprintf(buf, sizeof(buf), "%.2f V", voltaje_medir());
    return String(buf);
  }
  case MODO_RESISTENCIA:
  {
    float r = resistencia_medir();
    if (isnan(r))
      return "abierto";
    if (r >= 1000.0f)
      snprintf(buf, sizeof(buf), "%.2f kOhm", r / 1000.0f);
    else
      snprintf(buf, sizeof(buf), "%.0f Ohm", r);
    return String(buf);
  }
  case MODO_CAPACITANCIA:
  {
    float c = capacitancia_medir();
    if (isnan(c))
      return "abierto";
    if (c < 0.001f)
      snprintf(buf, sizeof(buf), "%.0f pF", c * 1000000.0f);
    else if (c < 1.0f)
      snprintf(buf, sizeof(buf), "%.2f nF", c * 1000.0f);
    else
      snprintf(buf, sizeof(buf), "%.2f uF", c);
    return String(buf);
  }
  case MODO_FRECUENCIA:
  {
    snprintf(buf, sizeof(buf), "%.0f Hz", frecuencia_medir());
    return String(buf);
  }
  case MODO_CONTINUIDAD:
    return continuidad_medir() ? "CONTINUIDAD" : "abierto";
  }
  return "--";
}

// ---------------- Setup ----------------
void setup()
{
  Serial.begin(115200);
  delay(1500);
  Serial.println("\n=== MULTIMETRO INICIADO ===");

  adc_init();                    // el ADC lo usan varios modos
  reles_setup();                 // todos los reles OFF
  seleccionar_modo(modo_activo); // conecta el front-end inicial
  delay(50);
  init_modo(modo_activo); // inicializa ese modo

  generador_setup(); // inicializa el generador de funciones (Fase 2)
  servidor_setup();  // levanta el AP y el servidor web
}

// ---------------- Loop ----------------
void loop()
{
  servidor_loop(); // atiende peticiones de la app
  delay(10);
}