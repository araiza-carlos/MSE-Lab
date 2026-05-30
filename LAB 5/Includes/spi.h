/******************************************************************************
 * Copyright (C) 2026 by Carlos Villarreal - CETYS Universidad
 *
 * Redistribution, modification or use of this software in source or binary
 * forms is permitted as long as the files maintain this copyright. Users are
 * permitted to modify this and use it to learn about the field of embedded
 * software. Carlos Villarreal and CETYS Universidad are not liable for any
 * misuse of this material.
 *
 *****************************************************************************/
/**
 * @file    spi.h
 * @brief   SPI Driver for STM32F411RE - Header
 *
 * Low-level SPI1 driver that configures the SPI peripheral for full-duplex
 * master communication using direct register access (no HAL/stdlib).
 *
 * Operating mode: Master, full-duplex, 8-bit, software CS.
 * Clock mode: CPOL=1, CPHA=1 (Mode 3) — SCK idle HIGH, capture on falling edge.
 * Compatible with ADXL345 SPI interface (requires CPOL=1, CPHA=1).
 *
 * Pin mapping (Nucleo-F411RE, SPI1, AF5):
 *   PA4 -> CS   (GPIO output, software-controlled)
 *   PA5 -> SCK  (AF5)
 *   PA6 -> MISO (AF5)
 *   PA7 -> MOSI (AF5)
 *
 * @author
 * @date    05/2026
 */

#ifndef __SPI_H__
#define __SPI_H__

/*** Includes ***/
#include <stdint.h>
#include "stm32f4xx.h"
#include "gpio_driver.h"

/*** Preprocessor Definitions ***/

/** SPI1 GPIO port */
#define SPI_GPIO_PORT       GPIOA

/** SPI1 pin assignments */
#define SPI_PIN_CS          GPIO_PIN_4   /**< Chip select (software GPIO)   */
#define SPI_PIN_SCK         GPIO_PIN_5   /**< Serial clock                  */
#define SPI_PIN_MISO        GPIO_PIN_6   /**< Master in, slave out          */
#define SPI_PIN_MOSI        GPIO_PIN_7   /**< Master out, slave in          */

/** Alternate function for SPI1 on PA5/PA6/PA7 */
#define SPI_GPIO_AF         5U

/** Timeout counter for polling loops */
#define SPI_TIMEOUT         100000U

/**
 * @brief SPI baud rate prescaler values.
 *
 * fSCK = fAPB2 / prescaler. With APB2 = 16 MHz (HSI, no PLL):
 *   SPI_BAUD_DIV2   ->  8 MHz
 *   SPI_BAUD_DIV4   ->  4 MHz
 *   SPI_BAUD_DIV8   ->  2 MHz  (recommended for ADXL345)
 *   SPI_BAUD_DIV16  ->  1 MHz
 *   SPI_BAUD_DIV32  ->  500 kHz
 *   SPI_BAUD_DIV64  ->  250 kHz
 *   SPI_BAUD_DIV128 ->  125 kHz
 *   SPI_BAUD_DIV256 ->  62.5 kHz
 */
#define SPI_BAUD_DIV2       0U
#define SPI_BAUD_DIV4       1U
#define SPI_BAUD_DIV8       2U
#define SPI_BAUD_DIV16      3U
#define SPI_BAUD_DIV32      4U
#define SPI_BAUD_DIV64      5U
#define SPI_BAUD_DIV128     6U
#define SPI_BAUD_DIV256     7U

/*** Type Definitions ***/

/**
 * @brief Return status codes for the SPI driver.
 */
typedef enum
{
    SPI_OK            = 0,  /**< Operation successful              */
    SPI_ERROR         = 1,  /**< Generic error                     */
    SPI_TIMEOUT_ERROR = 2,  /**< Polling loop timed out            */
    SPI_INVALID       = 3   /**< Invalid parameter (NULL pointer)  */
} spi_status_t;

/*** Function Prototypes ***/

/**
 * @brief  Initializes the SPI1 peripheral.
 *
 * Performs the following steps (FR-1, FR-2):
 *   - Enables GPIOA and SPI1 peripheral clocks.
 *   - Configures PA5 (SCK), PA6 (MISO), PA7 (MOSI) as AF5.
 *   - Configures PA4 as push-pull GPIO output for CS (idle HIGH).
 *   - Sets SPI1 to Master mode, CPOL=1, CPHA=1, 8-bit, software NSS.
 *   - Applies the specified baud rate prescaler.
 *   - Enables SPI1 peripheral (SPE bit).
 *
 * @param  baud_div  Clock prescaler (SPI_BAUD_DIVx). Use SPI_BAUD_DIV8
 *                   for 2 MHz, which is safe for most sensors.
 * @return SPI_OK always.
 */
spi_status_t spi_init(uint8_t baud_div);

/**
 * @brief  Transmits multiple bytes over the SPI bus (FR-3, FR-4).
 *
 * Sends `len` bytes from `tx_buf` in full-duplex mode. Received bytes
 * are discarded. CS must be managed externally with spi_csEnable /
 * spi_csDisable.
 *
 * @param  tx_buf  Pointer to the buffer containing data to transmit.
 *                 Must not be NULL.
 * @param  len     Number of bytes to transmit. Must be > 0.
 * @return SPI_OK on success, SPI_INVALID if tx_buf is NULL or len is 0,
 *         SPI_TIMEOUT_ERROR if the bus stalls.
 */
spi_status_t spi_transmit(const uint8_t *tx_buf, uint16_t len);

/**
 * @brief  Receives multiple bytes over the SPI bus (FR-5, FR-6).
 *
 * Reads `len` bytes into `rx_buf` by sending dummy bytes (0xFF) to
 * generate the clock. CS must be managed externally.
 *
 * @param  rx_buf  Pointer to the buffer where received data is stored.
 *                 Must not be NULL.
 * @param  len     Number of bytes to receive. Must be > 0.
 * @return SPI_OK on success, SPI_INVALID if rx_buf is NULL or len is 0,
 *         SPI_TIMEOUT_ERROR if the bus stalls.
 */
spi_status_t spi_receive(uint8_t *rx_buf, uint16_t len);

/**
 * @brief  Drives the CS line LOW to select the target device (FR-7).
 *
 * Call before starting any SPI transaction.
 */
void spi_csEnable(void);

/**
 * @brief  Drives the CS line HIGH to deselect the target device (FR-8).
 *
 * Call after the SPI transaction is complete.
 */
void spi_csDisable(void);

/**
 * @brief  Transmits and receives one byte simultaneously (full-duplex).
 *
 * Utility function used internally by spi_transmit and spi_receive.
 * Can also be called directly for register-level access to sensors.
 *
 * @param  tx_data  Byte to send.
 * @param  rx_data  Pointer to store the received byte. NULL = discard.
 * @return SPI_OK or SPI_TIMEOUT_ERROR.
 */
spi_status_t spi_transferByte(uint8_t tx_data, uint8_t *rx_data);

#endif /* __SPI_H__ */