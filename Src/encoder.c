#include "encoder.h"

extern TIM_HandleTypeDef htim1; // Right encoder
extern TIM_HandleTypeDef htim3; // Left encoder

void ENCODER_Init(void)
{
    // Start both encoders
    HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);

    // Reset counters
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    __HAL_TIM_SET_COUNTER(&htim3, 0);
}

void ENCODER_ResetLeft(void)
{
    __HAL_TIM_SET_COUNTER(&htim3, 0);
}

void ENCODER_ResetRight(void)
{
    __HAL_TIM_SET_COUNTER(&htim1, 0);
}

int32_t ENCODER_GetLeft(void)
{
    return (int16_t)__HAL_TIM_GET_COUNTER(&htim3);
}

int32_t ENCODER_GetRight(void)
{
    return (int16_t)__HAL_TIM_GET_COUNTER(&htim1);
}
