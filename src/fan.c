#include "fan.h"

// Use TIM2, Channel 2 (PA1) for PWM
// APB1 Timer Clock = 72MHz.
// Prescaler = 72-1 -> 1MHz counter.
// Period = 1000-1 -> 1kHz PWM frequency.
// Pulse = 0-1000 (0-100%).

TIM_HandleTypeDef htim2;

void MX_TIM2_Init(void) {
    TIM_MasterConfigTypeDef sMasterConfig = {0};
    TIM_OC_InitTypeDef sConfigOC = {0};

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 71;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 999;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) {
        Error_Handler();
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) {
        Error_Handler();
    }

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2) != HAL_OK) {
        Error_Handler();
    }
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* tim_pwmHandle) {
    if(tim_pwmHandle->Instance==TIM2) {
        __HAL_RCC_TIM2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        // PA1 -> TIM2_CH2
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    }
}

void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* tim_pwmHandle) {
    if(tim_pwmHandle->Instance==TIM2) {
        __HAL_RCC_TIM2_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1);
    }
}

void Fan_Init(void) {
    MX_TIM2_Init();
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
}

void Fan_SetSpeed(uint8_t percent) {
    if (percent > 100) percent = 100;

    // Pulse width is 0-999 (plus 1 -> 1000 ticks)
    // 100% -> 1000
    // 0% -> 0
    uint32_t pulse = (uint32_t)percent * 10;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, pulse);
}

void Fan_Update(float current_temp, float min_temp, float max_temp) {
    if (current_temp == -999.0f) {
        // Sensor Error: Fan 100%
        Fan_SetSpeed(100);
        return;
    }

    if (current_temp <= min_temp) {
        Fan_SetSpeed(0);
    } else if (current_temp >= max_temp) {
        Fan_SetSpeed(100);
    } else {
        // Linear interpolation
        // (temp - min) / (max - min) * 100
        float denominator = max_temp - min_temp;
        if (denominator < 0.1f) {
            // Avoid division by zero or weird behavior if min ~= max
            // If temp is above min (checked by previous else if), it's >= max too basically.
            Fan_SetSpeed(100);
        } else {
            float fraction = (current_temp - min_temp) / denominator;
            uint8_t speed = (uint8_t)(fraction * 100.0f);
            Fan_SetSpeed(speed);
        }
    }
}
