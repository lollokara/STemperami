#ifndef __UART_H
#define __UART_H

#include "main.h"

void MX_USART1_UART_Init(void);
void UART_Print(const char *str);
void UART_PrintLn(const char *str);
int UART_Available(void);
uint8_t UART_Read(void);

// Handle for UART
extern UART_HandleTypeDef huart1;

#endif /* __UART_H */
