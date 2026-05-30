/**
 * @file    sensor.h
 * @brief   ADXL345 Accelerometer Sensor Driver (I2C interface)
 * @author  Vania Leal
 * @date    May 2026
 *
 * Implements sensor_init, sensor_readData, and sensor_config
 * as required by Lab Assignment 05.
 *
 * I2C Address:
 *   SDO/ALT ADDRESS pin = GND → 0x53
 *   SDO/ALT ADDRESS pin = VCC → 0x1D
 *   Default used here: 0x53 (SDO tied to GND)
 */

#ifndef SENSOR_H
#define SENSOR_H

#include <stdint.h>

/* ─── I2C Address ──────────────────────────────────────────────────────────── */
#define ADXL345_I2C_ADDR      0x53U   /* SDO = GND */

/* ─── Register Map ─────────────────────────────────────────────────────────── */
#define ADXL345_REG_DEVID         0x00U   /* Device ID (should return 0xE5) */
#define ADXL345_REG_THRESH_TAP    0x1DU
#define ADXL345_REG_OFSX          0x1EU
#define ADXL345_REG_OFSY          0x1FU
#define ADXL345_REG_OFSZ          0x20U
#define ADXL345_REG_DUR           0x21U
#define ADXL345_REG_LATENT        0x22U
#define ADXL345_REG_WINDOW        0x23U
#define ADXL345_REG_THRESH_ACT    0x24U
#define ADXL345_REG_THRESH_INACT  0x25U
#define ADXL345_REG_TIME_INACT    0x26U
#define ADXL345_REG_ACT_INACT_CTL 0x27U
#define ADXL345_REG_TAP_AXES      0x2AU
#define ADXL345_REG_BW_RATE       0x2CU
#define ADXL345_REG_POWER_CTL     0x2DU
#define ADXL345_REG_INT_ENABLE    0x2EU
#define ADXL345_REG_INT_MAP       0x2FU
#define ADXL345_REG_INT_SOURCE    0x30U
#define ADXL345_REG_DATA_FORMAT   0x31U
#define ADXL345_REG_DATAX0        0x32U   /* LSB of X  (read 6 bytes for X,Y,Z) */
#define ADXL345_REG_DATAX1        0x33U
#define ADXL345_REG_DATAY0        0x34U
#define ADXL345_REG_DATAY1        0x35U
#define ADXL345_REG_DATAZ0        0x36U
#define ADXL345_REG_DATAZ1        0x37U
#define ADXL345_REG_FIFO_CTL      0x38U
#define ADXL345_REG_FIFO_STATUS   0x39U

/* ─── POWER_CTL bits ───────────────────────────────────────────────────────── */
#define ADXL345_POWER_MEASURE     (1U << 3)   /* Start measurement mode */
#define ADXL345_POWER_SLEEP       (1U << 2)
#define ADXL345_POWER_WAKEUP_8HZ  0x00U

/* ─── DATA_FORMAT bits ─────────────────────────────────────────────────────── */
#define ADXL345_RANGE_2G          0x00U
#define ADXL345_RANGE_4G          0x01U
#define ADXL345_RANGE_8G          0x02U
#define ADXL345_RANGE_16G         0x03U
#define ADXL345_FULL_RES          (1U << 3)   /* Full resolution mode */
#define ADXL345_DATA_JUSTIFY_RIGHT 0x00U

/* ─── BW_RATE bits ─────────────────────────────────────────────────────────── */
#define ADXL345_RATE_100HZ        0x0AU
#define ADXL345_RATE_50HZ         0x09U
#define ADXL345_RATE_25HZ         0x08U

/* ─── Device ID ────────────────────────────────────────────────────────────── */
#define ADXL345_DEVID             0xE5U

/* ─── Error Codes ──────────────────────────────────────────────────────────── */
#define ADXL345_OK                 0
#define ADXL345_ERR               -1
#define ADXL345_ERR_DEVID         -2

/* ─── Data Structure ───────────────────────────────────────────────────────── */
typedef struct {
    int16_t x;   /* Raw acceleration X-axis (LSB = 3.9 mg in ±2g full-res) */
    int16_t y;   /* Raw acceleration Y-axis */
    int16_t z;   /* Raw acceleration Z-axis */
} ADXL345_Data_t;

/* ─── Config Structure (for sensor_config) ─────────────────────────────────── */
typedef struct {
    uint8_t reg;
    uint8_t value;
} ADXL345_Config_t;

/* ─── API ──────────────────────────────────────────────────────────────────── */

/**
 * @brief  Initialize the ADXL345.
 *         Verifies DEVID, sets ±2g full-resolution, 100 Hz output rate,
 *         and enters measurement mode.
 * @return ADXL345_OK on success, ADXL345_ERR_DEVID or ADXL345_ERR on failure.
 */
int8_t sensor_init(void);

/**
 * @brief  Read X, Y, Z acceleration data from the ADXL345.
 * @param  data  Pointer to ADXL345_Data_t structure to populate.
 * @return ADXL345_OK on success, ADXL345_ERR on failure.
 */
int8_t sensor_readData(ADXL345_Data_t *data);

/**
 * @brief  Write a value to a specific ADXL345 register.
 * @param  reg    Register address (use ADXL345_REG_* defines).
 * @param  value  Byte to write.
 */
void sensor_config(uint8_t reg, uint8_t value);

#endif /* ADXL345_DRIVER_H */
