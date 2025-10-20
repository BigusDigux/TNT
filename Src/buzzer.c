#include "buzzer.h"

#define BUZZER_GPIO_PORT GPIOA
#define BUZZER_GPIO_PIN  GPIO_PIN_5

static TIM_HandleTypeDef *buzzer_htim = NULL;

//extern TIM_HandleTypeDef *buzzer_htim;

void delay_us(uint16_t us)
{
    __HAL_TIM_SET_COUNTER(buzzer_htim, 0);  // reset counter
    while (__HAL_TIM_GET_COUNTER(buzzer_htim) < us);
}

void playTone(uint16_t frequency, uint16_t duration_ms)
{
    uint32_t period_us = 1000000UL / frequency;
    uint32_t cycles = (frequency * duration_ms) / 1000;

    for (uint32_t i = 0; i < cycles; i++)
    {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        delay_us(period_us / 2);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        delay_us(period_us / 2);
    }
}


void Buzzer_Init(TIM_HandleTypeDef *htim) {
    buzzer_htim = htim;
    HAL_TIM_Base_Start(buzzer_htim); // Start TIM1
}

void Buzzer_Tick(void) {
    playTone(1000, 5);
}

void Buzzer_Short(void) {
    playTone(1000, 50);
}

void Buzzer_Confirm(void) {
    playTone(1800, 50);
    HAL_Delay(10);
    playTone(2200, 50);
}

void Buzzer_Startup(void) {
    playTone(1000, 50);
    HAL_Delay(10);
    playTone(1400, 50);
    HAL_Delay(10);
    playTone(1800, 50);
    HAL_Delay(10);
	playTone(0, 100);
}
