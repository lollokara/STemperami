#ifndef __ONEWIRE_H
#define __ONEWIRE_H

#include "main.h"

// Define 1-Wire Pin (e.g., PA0)
#define ONEWIRE_PIN GPIO_PIN_0
#define ONEWIRE_PORT GPIOA

void OneWire_Init(void);
uint8_t OneWire_Reset(void);
void OneWire_WriteByte(uint8_t data);
uint8_t OneWire_ReadByte(void);

#endif /* __ONEWIRE_H */
