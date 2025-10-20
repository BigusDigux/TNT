#ifndef __DRV8833_H
#define __DRV8833_H

#include "stm32f1xx_hal.h"

typedef struct {
    TIM_HandleTypeDef* htim;
    uint32_t channel_in1;
    uint32_t channel_in2;
    GPIO_TypeDef* in1_port;
    uint16_t in1_pin;
    GPIO_TypeDef* in2_port;
    uint16_t in2_pin;
} Motor_HandleTypeDef;

void DRV8833_Init(Motor_HandleTypeDef* motor);
void DRV8833_SetSpeed(Motor_HandleTypeDef* motor, int speed);
void DRV8833_Brake(Motor_HandleTypeDef* motor); 

#endif
