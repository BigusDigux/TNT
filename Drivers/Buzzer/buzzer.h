#ifndef __BUZZER_H__
#define __BUZZER_H__

#include "stm32f1xx_hal.h"

void Buzzer_Init(TIM_HandleTypeDef *htim);
void Buzzer_Tick(void);
void Buzzer_Short(void);
void Buzzer_Startup(void);
void Buzzer_Confirm(void);

#endif
