/**
 * @file    sensor.c
 * @brief   ADXL345 Accelerometer Sensor Driver Implementation
 * @author  Vania Leal
 * @date    May 2026
 *
 * Reads 6-byte burst (DATAX0–DATAZ1) via i2c_readRegDevice.
 * Writes single-byte registers via i2c_writeRegDevice.
 * The ADXL345 auto-increments the register address on burst reads.
 */

#include "sensor.h"
#include "i2c.h"

static int8_t _write_reg(uint8_t reg, uint8_t value)
{
    if (i2c_writeRegDevice(ADXL345_I2C_ADDR, reg, &value, 1U) != I2C_OK)
        return ADXL345_ERR;
    return ADXL345_OK;
}

/* ─── sensor_init ──────────────────────────────────────────────────────────── */
int8_t sensor_init(void)
{
    uint8_t devid = 0U;

    /* 1. Verify device identity */
    if (i2c_readRegDevice(ADXL345_I2C_ADDR, ADXL345_REG_DEVID, &devid, 1U) != I2C_OK)
        return ADXL345_ERR;

    if (devid != ADXL345_DEVID)
        return ADXL345_ERR_DEVID;

    /* 2. Put in standby before configuring */
    if (_write_reg(ADXL345_REG_POWER_CTL, 0x00U) != ADXL345_OK) return ADXL345_ERR;

    /* 3. Set output data rate: 100 Hz */
    if (_write_reg(ADXL345_REG_BW_RATE, ADXL345_RATE_100HZ) != ADXL345_OK) return ADXL345_ERR;

    /* 4. Set data format: ±2g, full resolution, right-justified */
    if (_write_reg(ADXL345_REG_DATA_FORMAT,
                   ADXL345_FULL_RES | ADXL345_RANGE_2G) != ADXL345_OK) return ADXL345_ERR;

    /* 5. Clear offsets */
    if (_write_reg(ADXL345_REG_OFSX, 0x00U) != ADXL345_OK) return ADXL345_ERR;
    if (_write_reg(ADXL345_REG_OFSY, 0x00U) != ADXL345_OK) return ADXL345_ERR;
    if (_write_reg(ADXL345_REG_OFSZ, 0x00U) != ADXL345_OK) return ADXL345_ERR;

    /* 6. Enter measurement mode */
    if (_write_reg(ADXL345_REG_POWER_CTL, ADXL345_POWER_MEASURE) != ADXL345_OK)
        return ADXL345_ERR;

    return ADXL345_OK;
}

/* ─── sensor_config ────────────────────────────────────────────────────────── */
void sensor_config(uint8_t reg, uint8_t value)
{
    _write_reg(reg, value);
}

/* ─── sensor_readData ──────────────────────────────────────────────────────── */
/**
 * Burst-read 6 bytes starting at DATAX0 to get X, Y, Z.
 *
 * Register layout (little-endian):
 *   buf[0] = DATAX0 (LSB),  buf[1] = DATAX1 (MSB)
 *   buf[2] = DATAY0 (LSB),  buf[3] = DATAY1 (MSB)
 *   buf[4] = DATAZ0 (LSB),  buf[5] = DATAZ1 (MSB)
 */
int8_t sensor_readData(ADXL345_Data_t *data)
{
    if (data == NULL) return ADXL345_ERR;

    uint8_t buf[6] = {0U};

    if (i2c_readRegDevice(ADXL345_I2C_ADDR, ADXL345_REG_DATAX0, buf, 6U) != I2C_OK)
        return ADXL345_ERR;

    /* Reconstruct signed 16-bit values (little-endian) */
    data->x = (int16_t)((uint16_t)buf[1] << 8U | (uint16_t)buf[0]);
    data->y = (int16_t)((uint16_t)buf[3] << 8U | (uint16_t)buf[2]);
    data->z = (int16_t)((uint16_t)buf[5] << 8U | (uint16_t)buf[4]);

    return ADXL345_OK;
}