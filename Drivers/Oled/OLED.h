
#ifndef OLED_H
#define OLED_H

#include "stm32f1xx_hal.h"

extern I2C_HandleTypeDef  *OLED_hi2c;

void OLED_Init(void);
void OLED_Clear(void);
void OLED_Print(char *s, uint8_t col, uint8_t row);
void OLED_PrintInt(int16_t val, uint8_t row, uint8_t col);

#endif
