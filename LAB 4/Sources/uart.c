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
 * @file uart.c
 * @brief UART Driver Implementation for STM32F411RE
 *
 * Implements uart_init() and uart_write() using direct register access
 * on USART2. No HAL or stdlib dependencies.
 *
 * @author  Vania Leal
 * @date    05/06/2026
 */

/*** Includes ***/
#include "uart.h"
#include "gpio_driver.h"

/*** Preprocessor Definitions ***/

/* USART2 TX pin: PA2, Alternate Function 7 */
#define UART_TX_PORT    GPIOA
#define UART_TX_PIN     2U
#define UART_TX_AF      7U

/*** Function Definitions ***/

/**
 * @brief  Initializes USART2 for asynchronous TX at 115200 baud.
 *
 * FR-1: Enables the clock for the USART2 peripheral (APB1ENR).
 * FR-2: Configures the baud rate divider (BRR) based on system clock.
 * FR-3: Enables the transmitter (TE) and the USART peripheral (UE).
 *
 * The TX pin (PA2) is also configured as Alternate Function 7.
 *
 * @return UART_OK always.
 */
uart_status_t uart_init(void)
{
    /* --- GPIO Configuration ---
     * Enable GPIOA clock and set PA2 to Alternate Function 7 (USART2_TX)
     */
    gpio_initPort(UART_TX_PORT);
    gpio_setAlternateFunction(UART_TX_PORT, UART_TX_PIN, UART_TX_AF);

    /* FR-1: Enable USART2 peripheral clock (on APB1 bus) */
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    /* FR-2: Configure baud rate
     * BRR = fCK / baud_rate
     * With HSI at 16 MHz and no prescaler on APB1:
     * BRR = 16,000,000 / 115,200 = ~138.88 -> 139 (0x008B)
     */
    USART2->BRR = (uint32_t)(UART_SYSTEM_CLOCK_HZ / UART_BAUD_RATE);

    /* FR-3: Enable transmitter (TE) and USART peripheral (UE) */
    USART2->CR1 = USART_CR1_TE | USART_CR1_UE;

    return UART_OK;
}

/**
 * @brief  Transmits a single byte over USART2.
 *
 * FR-4: Polls the TXE (Transmit Data Register Empty) flag in SR until set.
 * FR-5: Writes the 8-bit character to the USART2 data register (DR).
 *
 * @param  data  Byte to send.
 * @return UART_OK always.
 */
uart_status_t uart_write(uint8_t data)
{
    /* FR-4: Wait until the Transmit Data Register is empty */
    while (!(USART2->SR & USART_SR_TXE))
    {
        /* Busy-wait */
    }

    /* FR-5: Write the byte to the data register */
    USART2->DR = (uint32_t)(data & 0xFFU);

    return UART_OK;
}
