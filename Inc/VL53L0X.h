#ifndef __VL53L0X_H
#define __VL53L0X_H

#include "stm32f1xx_hal.h"

// I2C handle
extern I2C_HandleTypeDef hi2c1;

// Default I2C address
#define VL53L0X_DEFAULT_ADDR 0x52  // 7-bit <<1 = 0x52 for HAL

typedef struct {
    uint8_t address;       // I2C address
    GPIO_TypeDef* xshutPort;
    uint16_t xshutPin;
    uint8_t isInitialized;
} VL53L0X_HandleTypeDef;

// Initialize sensor, returns HAL_OK on success
HAL_StatusTypeDef VL53L0X_Init(VL53L0X_HandleTypeDef *dev);

// Read range in mm, returns HAL_OK on success
HAL_StatusTypeDef VL53L0X_ReadRange(VL53L0X_HandleTypeDef *dev, uint16_t *range_mm);

// Initialize multiple sensors sequentially with XSHUT pins
HAL_StatusTypeDef VL53L0X_InitSensors(VL53L0X_HandleTypeDef *sensors, uint8_t count);

#endif
