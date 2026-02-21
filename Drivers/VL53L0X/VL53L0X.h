#ifndef __VL53L0X_H
#define __VL53L0X_H

#include "stm32f1xx_hal.h"

/* Default I2C address (8-bit, HAL format) */
#define VL53L0X_DEFAULT_ADDR  0x52

typedef struct {
    uint8_t address;
    GPIO_TypeDef *xshutPort;
    uint16_t xshutPin;
    uint8_t isInitialized;
} VL53L0X_HandleTypeDef;

/* Basic API */
HAL_StatusTypeDef VL53L0X_Init(VL53L0X_HandleTypeDef *dev);
HAL_StatusTypeDef VL53L0X_ReadRange(VL53L0X_HandleTypeDef *dev, uint16_t *range_mm);
HAL_StatusTypeDef VL53L0X_ReadFiltered(VL53L0X_HandleTypeDef *dev, uint8_t id);
HAL_StatusTypeDef VL53L0X_InitSensors(VL53L0X_HandleTypeDef *sensors, uint8_t count);

/* Interrupt / continuous mode */
HAL_StatusTypeDef VL53L0X_EnableInterrupt(VL53L0X_HandleTypeDef *dev);
HAL_StatusTypeDef VL53L0X_StartContinuous(VL53L0X_HandleTypeDef *dev, uint16_t period_ms);
HAL_StatusTypeDef VL53L0X_ClearInterrupt(VL53L0X_HandleTypeDef *dev);

/* Init all*/
HAL_StatusTypeDef VL53L0X_AllInit(VL53L0X_HandleTypeDef *sensors, uint8_t count);

int16_t Check_Side(uint16_t left, uint16_t right);
#endif
