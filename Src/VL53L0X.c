#include "VL53L0X.h"
#include "stm32f1xx_hal.h"
#include <string.h>
#include <unistd.h> // optional for HAL_Delay

extern I2C_HandleTypeDef hi2c1;

#define VL53L0X_REG_IDENTIFICATION_MODEL_ID 0xC0
#define VL53L0X_REG_SYSRANGE_START 0x00
#define VL53L0X_REG_RESULT_RANGE_STATUS 0x14

static HAL_StatusTypeDef VL53L0X_SetAddress(VL53L0X_HandleTypeDef *dev, uint8_t new_addr) {
    uint8_t buf[2];
    buf[0] = 0x8A; // I2C slave device address register
    buf[1] = new_addr >> 1; // HAL expects 7-bit
    return HAL_I2C_Master_Transmit(&hi2c1, VL53L0X_DEFAULT_ADDR, buf, 2, 10);
}

HAL_StatusTypeDef VL53L0X_Init(VL53L0X_HandleTypeDef *dev) {
    if (!dev) return HAL_ERROR;

    // Wake sensor
    HAL_GPIO_WritePin(dev->xshutPort, dev->xshutPin, GPIO_PIN_SET);
    HAL_Delay(10);

    // Check sensor ID
    uint8_t id;
    if (HAL_I2C_Mem_Read(&hi2c1, VL53L0X_DEFAULT_ADDR, VL53L0X_REG_IDENTIFICATION_MODEL_ID, 1, &id, 1, 100) != HAL_OK) {
        dev->isInitialized = 0;
        return HAL_ERROR;
    }

    // Optionally change I2C address
    if (dev->address != VL53L0X_DEFAULT_ADDR) {
        if (VL53L0X_SetAddress(dev, dev->address) != HAL_OK) {
            dev->isInitialized = 0;
            return HAL_ERROR;
        }
    }

    dev->isInitialized = 1;
    return HAL_OK;
}

HAL_StatusTypeDef VL53L0X_ReadRange(VL53L0X_HandleTypeDef *dev, uint16_t *range_mm) {
    if (!dev || !range_mm || !dev->isInitialized) return HAL_ERROR;

    // Start single measurement
    uint8_t cmd = 0x01;
    if (HAL_I2C_Mem_Write(&hi2c1, dev->address, VL53L0X_REG_SYSRANGE_START, 1, &cmd, 1, 100) != HAL_OK)
        return HAL_ERROR;

    // Wait for completion (~20ms typical)
    HAL_Delay(10);

    // Read 2 bytes distance
    uint8_t buf[2];
    if (HAL_I2C_Mem_Read(&hi2c1, dev->address, 0x1E, 1, buf, 2, 100) != HAL_OK)
        return HAL_ERROR;

    *range_mm = ((uint16_t)buf[0] << 8) | buf[1];
    return HAL_OK;
}

HAL_StatusTypeDef VL53L0X_InitSensors(VL53L0X_HandleTypeDef *sensors, uint8_t count) {
    if (!sensors) return HAL_ERROR;

    // Reset all sensors
    for (uint8_t i = 0; i < count; i++) {
        HAL_GPIO_WritePin(sensors[i].xshutPort, sensors[i].xshutPin, GPIO_PIN_RESET);
        sensors[i].isInitialized = 0;
    }
    HAL_Delay(10);

    // Initialize sequentially to assign different addresses
    for (uint8_t i = 0; i < count; i++) {
        if (VL53L0X_Init(&sensors[i]) != HAL_OK)
            return HAL_ERROR;
        HAL_Delay(10);
    }
    return HAL_OK;
}
