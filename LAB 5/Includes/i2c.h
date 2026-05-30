/******************************************************************************
 * Copyright (C) 2026 by Carlos Villarreal - CETYS Universidad
 *****************************************************************************/
/**
 * @file    i2c.h
 * @brief   I2C Driver Header for STM32F411RE
 *
 * Bare-metal I2C1 driver using direct register access.
 * Configured for standard mode (100 kHz).
 *
 * Pins:
 *   PB8 -> I2C1_SCL (AF4, open-drain)
 *   PB9 -> I2C1_SDA (AF4, open-drain)
 *
 * @author  Vania Leal
 * @date    05/15/2026
 *
 * @ref     IEEE Std 830-1998
 * @ref     STM32F411RE Datasheet
 * @ref     RM0383 STM32F411xC/E Reference Manual
 */

#ifndef __I2C_H__
#define __I2C_H__

/*** Includes ***/
#include <stdint.h>
#include <stddef.h>
#include "stm32f4xx.h"

/*** Definitions ***/
#define I2C_TIMEOUT    100000U

/*** Type Definitions ***/

/**
 * @brief  Driver status codes returned by every public function.
 *         NFR-2: error handling for invalid args and bus errors.
 */
typedef enum
{
    I2C_OK             =  0,  /**< Operation succeeded                    */
    I2C_ERROR          = -1,  /**< Generic bus error                      */
    I2C_BUSY           = -2,  /**< Bus not idle at START                  */
    I2C_TIMEOUT_ERROR  = -3,  /**< Flag wait exceeded timeout limit       */
    I2C_INVALID        = -4   /**< NULL pointer or illegal parameter      */
} i2c_status_t;

/*** Public Functions ***/

/**
 * @brief  Initializes the I2C subsystem (FR-1, FR-2).
 *
 * Configures I2C1 to standard mode 100 kHz @ 16 MHz APB1.
 * Configures PB8 (SCL) and PB9 (SDA) as AF4, open-drain.
 * Requires external 4.7 kΩ pull-up resistors to 3.3 V.
 *
 * @return I2C_OK on success.
 */
i2c_status_t i2c_init(void);

/**
 * @brief  Writes data to a specific register of an I2C device (FR-3, FR-4).
 *
 * Sequence: START → addr(W) → reg_addr → data[0..len-1] → STOP
 *
 * @param  dev_addr  7-bit device address (not shifted).
 * @param  reg_addr  Register address inside the device.
 * @param  data      Pointer to data buffer to transmit.
 * @param  len       Number of bytes to write.
 * @return I2C_OK on success, I2C_INVALID if data is NULL,
 *         I2C_BUSY / I2C_TIMEOUT_ERROR on bus failure.
 */
i2c_status_t i2c_writeRegDevice(uint8_t dev_addr, uint8_t reg_addr,
                                 const uint8_t *data, size_t len);

/**
 * @brief  Writes data directly to an I2C device without a register (FR-5, FR-6).
 *
 * Sequence: START → addr(W) → data[0..len-1] → STOP
 *
 * @param  dev_addr  7-bit device address (not shifted).
 * @param  data      Pointer to data buffer to transmit.
 * @param  len       Number of bytes to write.
 * @return I2C_OK on success, I2C_INVALID if data is NULL,
 *         I2C_BUSY / I2C_TIMEOUT_ERROR on bus failure.
 */
i2c_status_t i2c_writeDevice(uint8_t dev_addr, const uint8_t *data, size_t len);

/**
 * @brief  Reads data from a specific register of an I2C device (FR-7, FR-8).
 *
 * Sequence: START → addr(W) → reg_addr → RESTART → addr(R) → buf[0..len-1] → STOP
 *
 * @param  dev_addr  7-bit device address (not shifted).
 * @param  reg_addr  Register address inside the device.
 * @param  buf       Buffer to store received bytes.
 * @param  len       Number of bytes to read.
 * @return I2C_OK on success, I2C_INVALID if buf is NULL,
 *         I2C_BUSY / I2C_TIMEOUT_ERROR on bus failure.
 */
i2c_status_t i2c_readRegDevice(uint8_t dev_addr, uint8_t reg_addr,
                                uint8_t *buf, size_t len);

/**
 * @brief  Reads data directly from an I2C device without a register (FR-9, FR-10).
 *
 * Sequence: START → addr(R) → buf[0..len-1] → STOP
 *
 * @param  dev_addr  7-bit device address (not shifted).
 * @param  buf       Buffer to store received bytes.
 * @param  len       Number of bytes to read.
 * @return I2C_OK on success, I2C_INVALID if buf is NULL,
 *         I2C_BUSY / I2C_TIMEOUT_ERROR on bus failure.
 */
i2c_status_t i2c_readDevice(uint8_t dev_addr, uint8_t *buf, size_t len);

#endif /* __I2C_H__ */