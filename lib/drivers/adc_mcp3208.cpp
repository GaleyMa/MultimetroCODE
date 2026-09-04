#include "adc_mcp3208.h"
#include "config.h"
#include <SPI.h>

static SPIClass spi(HSPI);

void adc_init()
{
    pinMode(PIN_ADC_CS, OUTPUT);
    digitalWrite(PIN_ADC_CS, HIGH);
    spi.begin(PIN_ADC_CLK, PIN_ADC_MISO, PIN_ADC_MOSI, PIN_ADC_CS);
    spi.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
}

uint16_t adc_read(uint8_t canal)
{
    uint8_t cmd = 0b00000110 | ((canal & 0x04) >> 2); // start + single-ended + bit alto canal
    uint8_t b1 = (canal & 0x03) << 6;                 // bits bajos de canal

    digitalWrite(PIN_ADC_CS, LOW);
    spi.transfer(cmd);
    uint8_t hi = spi.transfer(b1);
    uint8_t lo = spi.transfer(0x00);
    digitalWrite(PIN_ADC_CS, HIGH);

    return ((hi & 0x0F) << 8) | lo; // 12 bits
}

uint16_t adc_read_avg(uint8_t canal, int n)
{
    uint32_t acc = 0;
    for (int i = 0; i < n; i++)
    {
        acc += adc_read(canal);
        delayMicroseconds(200);
    }
    return acc / n;
}