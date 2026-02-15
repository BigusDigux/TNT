#include "MPU.h"
#include <math.h>

#define MPU_ADDR       (0x68 << 1)
#define ACCEL_CONFIG   0x1C
#define GYRO_CONFIG    0x1B
#define PWR_MGMT_1     0x6B
#define ACCEL_XOUT_H   0x3B
#define GYRO_XOUT_H    0x43

#define INT_ENABLE     0x38
#define INT_PIN_CFG    0x37
#define INT_STATUS     0x3A
#define CONFIG         0x1A
#define SMPLRT_DIV     0x19

I2C_HandleTypeDef* MPU_hi2c;

static float angle_z = 0;
static float gyro_z_offset = 0;

static uint32_t last_time = 0;
static int16_t gz;

/* ===== Internal: Read gyro Z ===== */
static void Read_MPU_GyroZ(void)
{
    uint8_t data[2];
    HAL_I2C_Mem_Read(MPU_hi2c, MPU_ADDR, GYRO_XOUT_H + 4, 1, data, 2, 100);
    gz = (int16_t)(data[0] << 8 | data[1]);
}

/* ===== Init ===== */
uint8_t MPU_Init(I2C_HandleTypeDef* hi2c)
{
    MPU_hi2c = hi2c;

    uint8_t check;
    HAL_I2C_Mem_Read(MPU_hi2c, MPU_ADDR, 0x75, 1, &check, 1, 100);
    if (check != 0x68) return 0;

    uint8_t data = 0x18;
    HAL_I2C_Mem_Write(MPU_hi2c, MPU_ADDR, PWR_MGMT_1, 1, &data, 1, 100);
    HAL_I2C_Mem_Write(MPU_hi2c, MPU_ADDR, ACCEL_CONFIG, 1, &data, 1, 100);
    HAL_I2C_Mem_Write(MPU_hi2c, MPU_ADDR, GYRO_CONFIG, 1, &data, 1, 100);

    last_time = HAL_GetTick();
    angle_z = 0;

    return 1;
}

/* ===== Enable DATA READY Interrupt ===== */
void MPU_EnableDataReadyInterrupt(void)
{
    uint8_t data;

    data = 0x00;  // DLPF 44Hz
    HAL_I2C_Mem_Write(MPU_hi2c, MPU_ADDR, CONFIG, 1, &data, 1, 100);

    data = 0x00;  // 200Hz
    HAL_I2C_Mem_Write(MPU_hi2c, MPU_ADDR, SMPLRT_DIV, 1, &data, 1, 100);

    data = 0x10;  // INT active high, latch until cleared
    HAL_I2C_Mem_Write(MPU_hi2c, MPU_ADDR, INT_PIN_CFG, 1, &data, 1, 100);

    data = 0x01;  // DATA READY
    HAL_I2C_Mem_Write(MPU_hi2c, MPU_ADDR, INT_ENABLE, 1, &data, 1, 100);
}

/* ===== Calibrate gyro ===== */
void MPU_CalibrateGyroZ(void)
{
    int32_t sum = 0;
    for (int i = 0; i < 1000; i++)
    {
        Read_MPU_GyroZ();
        sum += gz;
        HAL_Delay(2);
    }
    gyro_z_offset = sum / 1000.0f;
}

/* ===== Update (called from INT flag) ===== */
void MPU_Update(void)
{
    uint8_t status;
    HAL_I2C_Mem_Read(MPU_hi2c, MPU_ADDR, INT_STATUS, 1, &status, 1, 100);

    if (status & 0x01) { // Data Ready bit
		uint32_t now = HAL_GetTick();
		float dt = (now - last_time) / 1000.0f;
		last_time = now;

		Read_MPU_GyroZ();
		// Sensitivity for ±2000 deg/s is 16.4 LSB/deg/s
		float gyro_z_rate = (gz - gyro_z_offset) / 16.4f;

		angle_z += gyro_z_rate * dt;

		if (angle_z > 180) angle_z -= 360;
		else if (angle_z < -180) angle_z += 360;
    }
}

float MPU_GetYaw(void)
{
    return angle_z;
}
