#pragma once

// ---------- I2C (modo corriente - INA226) ----------
#define SDA_PIN 8
#define SCL_PIN 9
#define INA226_ADDR 0x40

// ---------- SPI (ADC MCP3208 - compartido) ----------
// Lo usan: voltaje, resistencia, capacitancia
#define PIN_ADC_CS 4
#define PIN_ADC_MOSI 5
#define PIN_ADC_CLK 6
#define PIN_ADC_MISO 7

// ---------- Referencia del ADC ----------
#define VREF 3.3f // 3.3V por ahora; sera 3.0V con la REF3030

// ---------- Calibracion CORRIENTE ----------
#define RSHUNT 0.1029f // calibrado contra fuente patrón
#define INA_SAT 32000  // umbral de saturacion (tope +-32767)

// ---------- Calibracion VOLTAJE (2 puntos, contra fuente de banco) ----------
#define V_CAL_BAJO 5.61f
#define RAW_CAL_BAJO 987.0f
#define V_CAL_ALTO 20.0f
#define RAW_CAL_ALTO 3483.0f

// ---------- Canales del ADC asignados a cada modo ----------
#define CH_VOLTAJE 0 // voltaje usa CH0
#define CH_RES_VD 1  // resistencia: Vd (drive) en CH1
#define CH_RES_VN 2  // resistencia: Vn (nodo)  en CH2

// ---------- Pines de control (pendientes / futuros) ----------
#define PIN_RES_DRIVE 14 // GPIO que alimenta el ohmetro (ratiometrico)

/// ---------- Resistencia (MUX CD74HC4051) ----------
#define PIN_MUX_S0 14
#define PIN_MUX_S1 15
#define PIN_MUX_S2 16
#define CH_RES_JUNCTION 2 // nodo común en CH2 del MCP3208

// ---------- Frecuencia (PCNT) ----------
#define PIN_FREC_IN 18  // entrada del contador (señal a medir)
#define PIN_FREC_GEN 17 // generador de prueba (quitar en versión final)

// ---------- Capacitancia ----------
#define PIN_CARGA_ALTA 21  // GPIO con la R de 1M  (para nF)
#define PIN_CARGA_BAJA 1   // GPIO con la R de 1k  (para uF) - ajusta a un pin libre
#define PIN_CAP_DESCARGA 2 // gate del 2N7000
#define CH_CAP 3           // nodo (via buffer) en CH3

// ---------- Pines de control de relés ----------
#define REL_CORRIENTE 10
#define REL_VOLTAJE 11
#define REL_RESIST 12
#define REL_CAPACIT 13
#define RELE_ON LOW
#define RELE_OFF HIGH