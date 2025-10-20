#include "drv8833.h"

void DRV8833_Init(Motor_HandleTypeDef* motor) {
    HAL_TIM_PWM_Start(motor->htim, motor->channel_in1);
    HAL_TIM_PWM_Start(motor->htim, motor->channel_in2);
}

void DRV8833_SetSpeed(Motor_HandleTypeDef* motor, int speed) {
    if (speed > 255) speed = 255;
    else if (speed < -255) speed = -255;

    if (speed > 0) {
        // Forward
        __HAL_TIM_SET_COMPARE(motor->htim, motor->channel_in1, 0);
        __HAL_TIM_SET_COMPARE(motor->htim, motor->channel_in2, speed);
    } else if (speed < 0) {
        // Backward
        __HAL_TIM_SET_COMPARE(motor->htim, motor->channel_in1, -speed);
        __HAL_TIM_SET_COMPARE(motor->htim, motor->channel_in2, 0);
    } else {
        // Stop
        __HAL_TIM_SET_COMPARE(motor->htim, motor->channel_in1, 0);
        __HAL_TIM_SET_COMPARE(motor->htim, motor->channel_in2, 0);
    }
}

void DRV8833_Brake(Motor_HandleTypeDef* motor) {
    // Apply full duty to both IN1 and IN2
    __HAL_TIM_SET_COMPARE(motor->htim, motor->channel_in1, 255);
    __HAL_TIM_SET_COMPARE(motor->htim, motor->channel_in2, 255);
}