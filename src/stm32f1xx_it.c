#include "stm32f1xx_it.h"
#include "stm32f1xx_hal.h"
#include "main.h" // For LED_PIN/PORT definitions

// Helper for Fault Blinking
static void Fault_Blink(void) {
    // Force Enable GPIOC Clock
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    // Configure PC13 as Output Push-Pull (if not already)
    // CRH Register: Pin 13 is bits 20-23.
    // Mode=10 (2MHz), CNF=00 (Push-Pull) -> 0010 = 0x2
    GPIOC->CRH &= ~(0xF << 20); // Clear
    GPIOC->CRH |= (0x2 << 20);  // Set 2MHz Output

    while (1) {
        GPIOC->BSRR = GPIO_PIN_13; // Set High (OFF for Active Low)
        for (volatile int i = 0; i < 500000; i++);
        GPIOC->BRR = GPIO_PIN_13;  // Set Low (ON for Active Low)
        for (volatile int i = 0; i < 500000; i++);
    }
}

void NMI_Handler(void) {
    Fault_Blink();
}

void HardFault_Handler(void) {
    Fault_Blink();
}

void MemManage_Handler(void) {
    Fault_Blink();
}

void BusFault_Handler(void) {
    Fault_Blink();
}

void UsageFault_Handler(void) {
    Fault_Blink();
}

void SVC_Handler(void) {
}

void DebugMon_Handler(void) {
}

void PendSV_Handler(void) {
}

void SysTick_Handler(void) {
    HAL_IncTick();
}

void USB_LP_CAN1_RX0_IRQHandler(void) {
    extern PCD_HandleTypeDef hpcd_USB_FS;
    HAL_PCD_IRQHandler(&hpcd_USB_FS);
}
