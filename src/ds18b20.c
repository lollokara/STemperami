#include "ds18b20.h"
#include "onewire.h"

void DS18B20_Init(void) {
    OneWire_Init();
}

void DS18B20_StartConversion(void) {
    if (OneWire_Reset()) {
        OneWire_WriteByte(0xCC); // Skip ROM
        OneWire_WriteByte(0x44); // Convert T
    }
}

float DS18B20_ReadTemp(void) {
    if (!OneWire_Reset()) {
        return -999.0f;
    }

    OneWire_WriteByte(0xCC); // Skip ROM
    OneWire_WriteByte(0xBE); // Read Scratchpad

    uint8_t low = OneWire_ReadByte();
    uint8_t high = OneWire_ReadByte();

    int16_t raw = (high << 8) | low;

    // Default resolution is 12-bit, divides by 16
    return (float)raw / 16.0f;
}

// Keeping the blocking one for compatibility if needed, but not used in main anymore
float DS18B20_GetTemp(void) {
    DS18B20_StartConversion();
    HAL_Delay(750);
    return DS18B20_ReadTemp();
}
