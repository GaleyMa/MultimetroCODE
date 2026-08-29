#include <Arduino.h>
#include <SPI.h>
#include <stdio.h>

#define PIN_CS 4
#define PIN_MOSI 5
#define PIN_CLK 6
#define PIN_MISO 7

const float VREF = 3.3; // por ahora; será 3.0 con la REF3030
const float V_ALTO = 19.98, RAW_ALTO = 2033.0;
const float V_BAJO = 2.08, RAW_BAJO = 210.0;

SPIClass spi(HSPI);

uint16_t readMCP3208(uint8_t canal)
{
  uint8_t cmd = 0b00000110 | ((canal & 0x04) >> 2); // start + single-ended + bit alto de canal
  uint8_t b1 = (canal & 0x03) << 6;                 // bits bajos de canal

  digitalWrite(PIN_CS, LOW);
  spi.transfer(cmd);
  uint8_t hi = spi.transfer(b1);
  uint8_t lo = spi.transfer(0x00);
  digitalWrite(PIN_CS, HIGH);

  return ((hi & 0x0F) << 8) | lo; // 12 bits
}

void setup()
{
  Serial.begin(115200);
  delay(1500);
  pinMode(PIN_CS, OUTPUT);
  digitalWrite(PIN_CS, HIGH);
  spi.begin(PIN_CLK, PIN_MISO, PIN_MOSI, PIN_CS);
  spi.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  Serial.println("MCP3208 listo. Leyendo CH0...");
}

void loop()
{
  uint16_t raw = readMCP3208(0);

  float v_entrada = V_BAJO + (raw - RAW_BAJO) * (V_ALTO - V_BAJO) / (RAW_ALTO - RAW_BAJO);

  if (v_entrada < 0)
  {
    v_entrada = 0;
  }

  Serial.printf("V_entrada = %.2f V  raw = %d \n", v_entrada, raw);
  delay(300);
}