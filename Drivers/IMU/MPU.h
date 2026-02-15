#ifndef __MPU_H
#define __MPU_H

#include "stm32f1xx_hal.h"

/* ================= MPU6050 API ================= */
uint8_t MPU_Init(I2C_HandleTypeDef* hi2c);

void MPU_EnableDataReadyInterrupt(void);
void MPU_CalibrateGyroZ(void);
void MPU_Update(void);
float MPU_GetYaw(void);

/* ================================================= */

#endif /* __MPU_H */
