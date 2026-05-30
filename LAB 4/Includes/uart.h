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
 * @file uart.h
 * @brief UART Driver for STM32F411RE - Header
 *
 * Low-level UART driver that configures USART2 for serial communication
 * at 115200 baud using direct register access (no HAL/stdlib).
 *
 * Pin mapping (Nucleo-F411RE):
 *   PA2 -> USART2_TX  (AF7)
 *   PA3 -> USART2_RX  (AF7)
 *
 * @author  Vania Leal
 * @date    05/06/2026
 */

#ifndef __UART_H__
#define __UART_H__

/*** Includes ***/
#include <stdint.h>
#include "stm32f4xx.h"

/*** Preprocessor Definitions ***/

/** System clock frequency (HSI = 16 MHz on STM32F411RE) */
#define UART_SYSTEM_CLOCK_HZ    16000000UL

/** Target baud rate */
#define UART_BAUD_RATE          115200UL

/*** Type Prototypes ***/

/**
 * @brief Return status codes for the UART driver.
 */
typedef enum {
    UART_OK      = 0,   /**< Operation successful   */
    UART_ERROR   = 1,   /**< Generic error          */
    UART_INVALID = 2    /**< Invalid parameter      */
} uart_status_t;

/*** Function Prototypes ***/

/**
 * @brief  Initializes the USART2 peripheral for TX at 115200 baud.
 *
 * Performs the following steps (FR-1 through FR-3):
 *  - Enables GPIOA and USART2 peripheral clocks.
 *  - Configures PA2 as Alternate Function 7 (USART2_TX).
 *  - Sets the baud rate divider based on the system clock.
 *  - Enables the transmitter (TE) and the USART peripheral (UE).
 *
 * @return UART_OK on success.
 */
uart_status_t uart_init(void);

/**
 * @brief  Transmits a single byte over USART2.
 *
 * Waits until the Transmit Data Register Empty (TXE) flag is set,
 * then writes the byte to the data register (FR-4, FR-5).
 *
 * @param  data   Byte to transmit.
 * @return UART_OK on success.
 */
uart_status_t uart_write(uint8_t data);

#endif /* __UART_H__ */
