#include "uart.h"
#include <string.h>
#include <stdio.h>

UART_HandleTypeDef huart1;
static uint8_t rx_buffer[1]; // Single byte buffer for interrupt
#define RING_BUFFER_SIZE 128
static volatile uint8_t ring_buffer[RING_BUFFER_SIZE];
static volatile uint16_t ring_head = 0;
static volatile uint16_t ring_tail = 0;

void MX_USART1_UART_Init(void) {
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler();
    }

    // Start Receive Interrupt
    HAL_UART_Receive_IT(&huart1, rx_buffer, 1);
}

void UART_Print(const char *str) {
    HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), 100);
}

void UART_PrintLn(const char *str) {
    UART_Print(str);
    UART_Print("\r\n");
}

int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, 100);
    return len;
}

// UART RX Complete Callback
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        // Store byte in ring buffer
        uint16_t next_head = (ring_head + 1) % RING_BUFFER_SIZE;
        if (next_head != ring_tail) {
            ring_buffer[ring_head] = rx_buffer[0];
            ring_head = next_head;
        }
        // Restart reception
        HAL_UART_Receive_IT(&huart1, rx_buffer, 1);
    }
}

// Check if data available
int UART_Available(void) {
    return (ring_head != ring_tail);
}

// Read byte from buffer
uint8_t UART_Read(void) {
    if (ring_head == ring_tail) return 0;

    uint8_t data = ring_buffer[ring_tail];
    ring_tail = (ring_tail + 1) % RING_BUFFER_SIZE;
    return data;
}

// HAL Hook for MSP Init
void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(uartHandle->Instance==USART1) {
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        // Enable NVIC for UART
        HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle) {
    if(uartHandle->Instance==USART1) {
        __HAL_RCC_USART1_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);
        HAL_NVIC_DisableIRQ(USART1_IRQn);
    }
}

// IRQ Handler (Must be defined to call HAL handler)
void USART1_IRQHandler(void) {
    HAL_UART_IRQHandler(&huart1);
}
