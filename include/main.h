#ifndef __MAIN_H
#define __MAIN_H

#include "stm32f1xx_hal.h"

// Pin Definitions
#define LED_PIN GPIO_PIN_13
#define LED_GPIO_PORT GPIOC

void Error_Handler(void);

#endif /* __MAIN_H */
