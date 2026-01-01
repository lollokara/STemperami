#include "onewire.h"

// Helper function to set pin as output
static void SetPinOutput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = ONEWIRE_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD; // Open Drain
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(ONEWIRE_PORT, &GPIO_InitStruct);
}

// Helper function to set pin as input
static void SetPinInput(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = ONEWIRE_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ONEWIRE_PORT, &GPIO_InitStruct);
}

// Microsecond delay
static void Delay_us(uint32_t us) {
    // Crude delay loop for 72MHz.
    // 1 us is roughly 72 cycles.
    // Each loop iteration takes about 4-5 cycles (varies by compiler).
    // Better to use DWT or Timer, but for now loop is sufficient for 1-Wire if calibrated.
    // Factor: 72MHz / 5 cycles ~ 14. But let's be conservative.
    // Calibration: with -O2, a simple loop is very fast.
    // Using a simple approximation:
    __IO uint32_t count = us * 8; // Tuned for generic GCC -Os/-O2 at 72MHz
    while (count--) {
        __NOP();
    }
}

void OneWire_Init(void) {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    SetPinOutput();
    HAL_GPIO_WritePin(ONEWIRE_PORT, ONEWIRE_PIN, GPIO_PIN_SET);
}

uint8_t OneWire_Reset(void) {
    uint8_t presence = 0;
    SetPinOutput();

    HAL_GPIO_WritePin(ONEWIRE_PORT, ONEWIRE_PIN, GPIO_PIN_RESET);
    Delay_us(480); // Min 480us

    HAL_GPIO_WritePin(ONEWIRE_PORT, ONEWIRE_PIN, GPIO_PIN_SET);
    Delay_us(80); // Wait for presence pulse (15-60us wait + 60-240us pulse)

    SetPinInput();
    if (HAL_GPIO_ReadPin(ONEWIRE_PORT, ONEWIRE_PIN) == GPIO_PIN_RESET) {
        presence = 1;
    } else {
        presence = 0;
    }

    Delay_us(400); // Complete the slot
    return presence;
}

void OneWire_WriteBit(uint8_t bit) {
    SetPinOutput();
    HAL_GPIO_WritePin(ONEWIRE_PORT, ONEWIRE_PIN, GPIO_PIN_RESET);

    if (bit) {
        Delay_us(10); // Write 1 start (1-15us)
        HAL_GPIO_WritePin(ONEWIRE_PORT, ONEWIRE_PIN, GPIO_PIN_SET);
        Delay_us(60); // Recovery
    } else {
        Delay_us(60); // Write 0 (60-120us)
        HAL_GPIO_WritePin(ONEWIRE_PORT, ONEWIRE_PIN, GPIO_PIN_SET);
        Delay_us(10); // Recovery
    }
}

uint8_t OneWire_ReadBit(void) {
    uint8_t bit = 0;
    SetPinOutput();
    HAL_GPIO_WritePin(ONEWIRE_PORT, ONEWIRE_PIN, GPIO_PIN_RESET);
    Delay_us(2); // Start read slot (>1us)

    HAL_GPIO_WritePin(ONEWIRE_PORT, ONEWIRE_PIN, GPIO_PIN_SET); // Release
    SetPinInput();
    Delay_us(10); // Wait valid data (sample near 15us from start)

    if (HAL_GPIO_ReadPin(ONEWIRE_PORT, ONEWIRE_PIN)) {
        bit = 1;
    }

    Delay_us(50); // Complete slot
    return bit;
}

void OneWire_WriteByte(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        OneWire_WriteBit(data & 0x01);
        data >>= 1;
    }
}

uint8_t OneWire_ReadByte(void) {
    uint8_t data = 0;
    for (int i = 0; i < 8; i++) {
        if (OneWire_ReadBit()) {
            data |= (1 << i);
        }
    }
    return data;
}
