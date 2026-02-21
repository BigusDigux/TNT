#include "VL53L0X.h"
#include "stm32f1xx_hal.h"
#include <string.h>
#include "system_config.h"

extern I2C_HandleTypeDef hi2c1;

#define VL53L0X_REG_IDENTIFICATION_MODEL_ID 0xC0
#define VL53L0X_REG_RESULT_RANGE_STATUS 0x14

#define VL53L0X_REG_RESULT_RANGE_MM  0x1E
#define VL53L0X_REG_SYSTEM_INTERRUPT_CONFIG_GPIO  0x0A
#define VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR        0x0B
#define VL53L0X_REG_SYSRANGE_INTERMEASUREMENT     0x04
#define VL53L0X_REG_SYSRANGE_START                0x00

// TOF config
extern uint16_t dist[4];
extern uint16_t WallDist_S, TargetDist_S, WallDist_F, TargetDist_F, WallDist_Fst, TargetDist_Fst;

#define TOF_COUNT 4

static uint16_t history[TOF_COUNT][3] = {0};

static uint16_t median3(uint16_t a, uint16_t b, uint16_t c)
{
    if (a > b) { uint16_t t = a; a = b; b = t; }
    if (b > c) { uint16_t t = b; b = c; c = t; }
    if (a > b) { uint16_t t = a; a = b; b = t; }
    return b;
}

HAL_StatusTypeDef VL53L0X_EnableInterrupt(VL53L0X_HandleTypeDef *dev)
{
    uint8_t cfg = 0x04; // New sample ready
    return HAL_I2C_Mem_Write(&hi2c1,
                             dev->address,
                             VL53L0X_REG_SYSTEM_INTERRUPT_CONFIG_GPIO,
                             1, &cfg, 1, 100);
}

HAL_StatusTypeDef VL53L0X_StartContinuous(VL53L0X_HandleTypeDef *dev, uint16_t period_ms)
{
    uint8_t buf[2];

    // Inter-measurement period
    buf[0] = (period_ms >> 8) & 0xFF;
    buf[1] = period_ms & 0xFF;

    HAL_I2C_Mem_Write(&hi2c1, dev->address,
                      VL53L0X_REG_SYSRANGE_INTERMEASUREMENT,
                      1, buf, 2, 100);

    uint8_t start = 0x02; // continuous mode
    return HAL_I2C_Mem_Write(&hi2c1, dev->address,
                             VL53L0X_REG_SYSRANGE_START,
                             1, &start, 1, 100);
}

HAL_StatusTypeDef VL53L0X_ClearInterrupt(VL53L0X_HandleTypeDef *dev)
{
    uint8_t clr = 0x01;
    return HAL_I2C_Mem_Write(&hi2c1, dev->address,
                             VL53L0X_REG_SYSTEM_INTERRUPT_CLEAR,
                             1, &clr, 1, 100);
}

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

HAL_StatusTypeDef VL53L0X_ReadRange(VL53L0X_HandleTypeDef *dev, uint16_t *range_mm)
{
    if (!dev || !range_mm || !dev->isInitialized)
        return HAL_ERROR;

    uint8_t buf[2];

    if (HAL_I2C_Mem_Read(&hi2c1, dev->address, VL53L0X_REG_RESULT_RANGE_MM, 1, buf, 2, 100) != HAL_OK)
        return HAL_ERROR;

    uint16_t raw = ((uint16_t)buf[0] << 8) | buf[1];

    if (raw == 0 || raw == 20)
        raw = 8190;

    *range_mm = raw;

    return HAL_OK;
}

HAL_StatusTypeDef VL53L0X_ReadFiltered(VL53L0X_HandleTypeDef *dev, uint8_t id)
{
    uint16_t raw;

    if (VL53L0X_ReadRange(dev, &raw) != HAL_OK)
        return HAL_ERROR;

    // Shift history
    history[id][0] = history[id][1];
    history[id][1] = history[id][2];
    history[id][2] = raw;

    // Apply median filter
    dist[id] = median3(history[id][0],
                       history[id][1],
                       history[id][2]);

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

HAL_StatusTypeDef VL53L0X_AllInit(VL53L0X_HandleTypeDef *sensors, uint8_t count)
{
    if (!sensors || count == 0) return HAL_ERROR;

    /* Reset + address assignment */
    if (VL53L0X_InitSensors(sensors, count) != HAL_OK)
        return HAL_ERROR;

    /* Enable interrupt + continuous ranging */
    for (uint8_t i = 0; i < count; i++) {
        if (VL53L0X_EnableInterrupt(&sensors[i]) != HAL_OK)
            return HAL_ERROR;

        if (VL53L0X_StartContinuous(&sensors[i], 30) != HAL_OK)
            return HAL_ERROR;
    }

    return HAL_OK;
}

int16_t Check_Side(uint16_t left, uint16_t right){
	if (left < WallDist_S && right < WallDist_S && left > 0 && right > 0){
		return left - right;
	}else if (left < WallDist_S && left > 0){
		return ((left - TargetDist_S) * 15) / 10;
	}else if (right < WallDist_S && right > 0){
		return ((TargetDist_S - right) * 15) / 10;
	} else return 0;
}
