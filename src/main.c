#include "main.h"
#include "serial_usb.h"
#include "usb_device.h"
#include "cli.h"
#include "flash_store.h"
#include "fan.h"
#include "ds18b20.h"
#include "stm32f1xx_it.h"

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_IWDG_Init(void);

// Global State
AppSettings current_settings;
float current_temperature = 0.0f;

IWDG_HandleTypeDef hiwdg;

// Temperature State Machine
typedef enum {
    TEMP_IDLE,
    TEMP_CONVERTING,
    TEMP_READING
} TempState;

int main(void) {
    // HAL Init
    HAL_Init();

    // Initialize GPIO first to allow LED status usage
    MX_GPIO_Init();
    // Turn LED ON (Active Low) to indicate boot start
    HAL_GPIO_WritePin(LED_GPIO_PORT, LED_PIN, GPIO_PIN_RESET);

    // USB Soft Disconnect: Pull PA12 Low for 10ms to force re-enumeration
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_Delay(10); // Simple delay
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_12); // Release pin for USB peripheral

    // Configure the system clock
    SystemClock_Config();

    // USB Device Init
    MX_USB_DEVICE_Init();

    // Fan Init (PWM)
    Fan_Init();
    // Checkpoint 1: 2 Blinks
    HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN); HAL_Delay(100); HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN); HAL_Delay(100);
    HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN); HAL_Delay(100); HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN); HAL_Delay(500);

    // OneWire / DS18B20 Init
    DS18B20_Init();
    // Checkpoint 2: 3 Blinks
    for(int i=0; i<3; i++) { HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN); HAL_Delay(100); HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN); HAL_Delay(100); }
    HAL_Delay(500);

    // Flash Store Init & Load
    FlashStore_Init();
    FlashStore_Load(&current_settings);
    // Checkpoint 3: 4 Blinks
    for(int i=0; i<4; i++) { HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN); HAL_Delay(100); HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN); HAL_Delay(100); }
    HAL_Delay(500);

    // CLI Init
    CLI_Init();

    // Watchdog Init
    // Disabled for debugging boot issues
    // MX_IWDG_Init();

    uint32_t last_tick = 0;
    uint32_t last_temp_trigger = 0;
    TempState temp_state = TEMP_IDLE;
    uint32_t temp_conversion_start = 0;

    while (1) {
        uint32_t now = HAL_GetTick();

        // Heartbeat LED (1s)
        if (now - last_tick >= 1000) {
            HAL_GPIO_TogglePin(LED_GPIO_PORT, LED_PIN);
            last_tick = now;
        }

        // Non-blocking Temperature State Machine
        // 1. Trigger every 2s
        if (temp_state == TEMP_IDLE) {
            if (now - last_temp_trigger >= 2000) {
                DS18B20_StartConversion();
                temp_state = TEMP_CONVERTING;
                temp_conversion_start = now;
            }
        }
        // 2. Wait for conversion (750ms for 12-bit)
        else if (temp_state == TEMP_CONVERTING) {
            if (now - temp_conversion_start >= 750) {
                current_temperature = DS18B20_ReadTemp();
                Fan_Update(current_temperature, current_settings.min_temp, current_settings.max_temp);
                last_temp_trigger = now;
                temp_state = TEMP_IDLE;
            }
        }

        // Process CLI
        CLI_Process();

        // Refresh Watchdog
        // HAL_IWDG_Refresh(&hiwdg);
    }
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // Initializes the RCC Oscillators according to the specified parameters
    // in the RCC_OscInitTypeDef structure.
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.LSIState = RCC_LSI_ON; // LSI for IWDG
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    // Initializes the CPU, AHB and APB buses clocks
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }

    // USB Clock Config (USB needs 48MHz)
    // PLL is 72MHz. 72 / 1.5 = 48MHz.
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
    PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        Error_Handler();
    }
}

static void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE(); // Enable AFIO for safety

    /*Configure GPIO pin Output Level */
    HAL_GPIO_WritePin(GPIOC, LED_PIN, GPIO_PIN_RESET);

    /*Configure GPIO pin : LED_PIN */
    GPIO_InitStruct.Pin = LED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

static void MX_IWDG_Init(void) {
    // LSI is 40kHz (approx)
    // Prescaler 32 -> 1.25kHz (0.8ms per tick)
    // Reload 4095 -> ~3.2 seconds
    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
    hiwdg.Init.Reload = 4095;
    if (HAL_IWDG_Init(&hiwdg) != HAL_OK) {
        Error_Handler();
    }
}

void Error_Handler(void) {
    __disable_irq();
    // Force Enable GPIOC Clock just in case we crashed before MX_GPIO_Init
    RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;

    // Configure PC13 as Output Push-Pull (if not already)
    // CRH Register: Pin 13 is bits 20-23. Mode=10 (2MHz), CNF=00 (Push-Pull) -> 0x2
    GPIOC->CRH &= ~(0xF << 20); // Clear
    GPIOC->CRH |= (0x2 << 20);  // Set 2MHz Output

    while (1) {
        // Fast Death Blink (Error State)
        GPIOC->BRR = GPIO_PIN_13; // ON
        for (volatile int i = 0; i < 100000; i++);
        GPIOC->BSRR = GPIO_PIN_13; // OFF
        for (volatile int i = 0; i < 100000; i++);
    }
}
