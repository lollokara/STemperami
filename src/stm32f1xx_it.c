#include "stm32f1xx_it.h"
#include "stm32f1xx_hal.h"

// External declaration for UART handle if needed by HAL_UART_IRQHandler
// However, HAL_UART_IRQHandler calls the instance directly, but we call HAL_UART_IRQHandler(&huart1) in uart.c
// The SysTick_Handler is called by the system.

void SysTick_Handler(void) {
    HAL_IncTick();
}

void USB_LP_CAN1_RX0_IRQHandler(void) {
    extern PCD_HandleTypeDef hpcd_USB_FS;
    HAL_PCD_IRQHandler(&hpcd_USB_FS);
}
