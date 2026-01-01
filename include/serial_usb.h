#ifndef __SERIAL_USB_H
#define __SERIAL_USB_H

#include "main.h"

void Serial_Print(const char *str);
void Serial_PrintLn(const char *str);
int Serial_Available(void);
uint8_t Serial_Read(void);

// Callback used by CDC Interface to push data
void Serial_RxCallback(uint8_t byte);

#endif /* __SERIAL_USB_H */
