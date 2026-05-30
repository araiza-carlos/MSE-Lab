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
 * @file    spi.c
 * @brief   SPI Driver Implementation for STM32F411RE
 *
 * Implements spi_init(), spi_transmit(), spi_receive(), spi_csEnable(),
 * and spi_csDisable() using direct register access on SPI1.
 * No HAL or stdlib dependencies.
 *
 * Design decisions:
 *   - CPOL=1, CPHA=1 (Mode 3): compatible with ADXL345 and most sensors
 *     that specify SCK idle HIGH with capture on the falling edge.
 *   - Software CS (SSM=1, SSI=1): gives full control over CS timing,
 *     which is required when the CS line must stay low across multiple
 *     byte transfers (e.g., multi-byte burst reads on the ADXL345).
 *   - 8-bit frame format (DFF=0): standard for sensor communication.
 *   - spi_transferByte() is the core primitive; spi_transmit() and
 *     spi_receive() are built on top of it.
 *   - The DR register is accessed via an 8-bit pointer cast to prevent
 *     the compiler from widening the write to 16 bits, which would
 *     corrupt the SPI frame on some GCC versions.
 *
 * @author Kheara Kieley
 * @date    05/2026
 */

/*** Includes ***/
#include "spi.h"

/* -------------------------------------------------------------------------
 * Private helper
 * ----------------------------------------------------------------------- */

/**
 * @brief  Core full-duplex byte transfer.
 *
 * Waits for TXE, writes tx_data, waits for RXNE, reads received byte.
 * Both TX and RX happen simultaneously because SPI is full-duplex.
 *
 * FR-4 (TX sync) and FR-6 (RX sync) are satisfied here by polling
 * the TXE and RXNE flags respectively before accessing DR.
 *
 * @param  tx_data  Byte to transmit.
 * @param  rx_data  Where to store the received byte. NULL = discard.
 * @return SPI_OK or SPI_TIMEOUT_ERROR.
 */
spi_status_t spi_transferByte(uint8_t tx_data, uint8_t *rx_data)
{
    uint32_t timeout;

    /* FR-4: Wait until Transmit Data Register is Empty (TXE = 1) */
    timeout = SPI_TIMEOUT;
    while (!(SPI1->SR & SPI_SR_TXE))
    {
        if (--timeout == 0U) return SPI_TIMEOUT_ERROR;
    }

    /*
     * Write 8-bit data to DR using a byte-width pointer.
     * Casting to __IO uint8_t * prevents GCC from generating a
     * 16-bit write, which would corrupt the SPI frame.
     */
    *(__IO uint8_t *)&SPI1->DR = tx_data;

    /* FR-6: Wait until Receive buffer Not Empty (RXNE = 1) */
    timeout = SPI_TIMEOUT;
    while (!(SPI1->SR & SPI_SR_RXNE))
    {
        if (--timeout == 0U) return SPI_TIMEOUT_ERROR;
    }

    /* Read received byte — clears RXNE flag */
    uint8_t rx = (uint8_t)SPI1->DR;
    if (rx_data != NULL)
    {
        *rx_data = rx;
    }

    return SPI_OK;
}

/* -------------------------------------------------------------------------
 * Public functions
 * ----------------------------------------------------------------------- */

/**
 * FR-1: Configure SPI peripheral to default state.
 * FR-2: Enable clocking for SPI hardware.
 */
spi_status_t spi_init(uint8_t baud_div)
{
    /* FR-2: Enable GPIOA clock (AHB1) */
    gpio_initPort(SPI_GPIO_PORT);

    /* --- Configure SCK (PA5), MISO (PA6), MOSI (PA7) as AF5 ----------- */
    gpio_setAlternateFunction(SPI_GPIO_PORT, SPI_PIN_SCK,  SPI_GPIO_AF);
    gpio_setAlternateFunction(SPI_GPIO_PORT, SPI_PIN_MISO, SPI_GPIO_AF);
    gpio_setAlternateFunction(SPI_GPIO_PORT, SPI_PIN_MOSI, SPI_GPIO_AF);

    /* SCK, MOSI: push-pull output type (not open-drain) */
    GPIOA->OTYPER &= ~((1U << (uint32_t)SPI_PIN_SCK)  |
                       (1U << (uint32_t)SPI_PIN_MOSI));

    /* SCK, MOSI, MISO: high speed */
    GPIOA->OSPEEDR |= (3U << ((uint32_t)SPI_PIN_SCK  * 2U)) |
                      (3U << ((uint32_t)SPI_PIN_MISO * 2U)) |
                      (3U << ((uint32_t)SPI_PIN_MOSI * 2U));

    /* MISO: no pull-up/pull-down (slave drives the line) */
    GPIOA->PUPDR &= ~(3U << ((uint32_t)SPI_PIN_MISO * 2U));

    /* --- Configure CS (PA4) as push-pull GPIO output, idle HIGH -------- */
    gpio_setPinMode(SPI_GPIO_PORT, SPI_PIN_CS, GPIO_MODE_OUTPUT);
    GPIOA->OTYPER  &= ~(1U << (uint32_t)SPI_PIN_CS);
    GPIOA->OSPEEDR |=  (3U << ((uint32_t)SPI_PIN_CS * 2U));
    spi_csDisable();   /* CS = HIGH (inactive) */

    /* FR-2: Enable SPI1 clock (APB2) */
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    /* FR-1: Reset CR1 and CR2 to a known state before configuring */
    SPI1->CR1 = 0U;
    SPI1->CR2 = 0U;

    /*
     * FR-1: Configure SPI1 default state:
     *
     *   BIDIMODE = 0  -> full-duplex, 2-line unidirectional
     *   DFF      = 0  -> 8-bit data frame
     *   SSM      = 1  -> software slave management (CS via GPIO)
     *   SSI      = 1  -> internal NSS = 1 (prevents mode fault)
     *   LSBFIRST = 0  -> MSB first (standard for most sensors)
     *   SPE      = 0  -> SPI disabled during configuration
     *   BR[2:0]       -> baud rate prescaler (user-supplied)
     *   MSTR     = 1  -> Master mode
     *   CPOL     = 1  -> SCK idle HIGH
     *   CPHA     = 1  -> data captured on second (falling) clock edge
     */
    SPI1->CR1 = SPI_CR1_SSM                              |
                SPI_CR1_SSI                              |
                SPI_CR1_MSTR                             |
                SPI_CR1_CPOL                             |
                SPI_CR1_CPHA                             |
                ((uint32_t)(baud_div & 0x07U) << SPI_CR1_BR_Pos);

    /* Enable SPI1 (SPE bit) */
    SPI1->CR1 |= SPI_CR1_SPE;

    return SPI_OK;
}

/**
 * FR-3: Send multiple bytes from tx_buf.
 * FR-4: Ensure synchronization with SPI clock (handled in spi_transferByte).
 */
spi_status_t spi_transmit(const uint8_t *tx_buf, uint16_t len)
{
    /* NFR-2: Error handling for invalid buffer pointer */
    if (tx_buf == NULL || len == 0U) return SPI_INVALID;

    for (uint16_t i = 0U; i < len; i++)
    {
        spi_status_t st = spi_transferByte(tx_buf[i], NULL);
        if (st != SPI_OK) return st;
    }

    return SPI_OK;
}

/**
 * FR-5: Read multiple bytes into rx_buf.
 * FR-6: Ensure synchronization with SPI clock (handled in spi_transferByte).
 */
spi_status_t spi_receive(uint8_t *rx_buf, uint16_t len)
{
    /* NFR-2: Error handling for invalid buffer pointer */
    if (rx_buf == NULL || len == 0U) return SPI_INVALID;

    for (uint16_t i = 0U; i < len; i++)
    {
        /*
         * Send dummy byte 0xFF to generate the clock pulses needed
         * for the slave to shift out its data on MISO.
         */
        spi_status_t st = spi_transferByte(0xFFU, &rx_buf[i]);
        if (st != SPI_OK) return st;
    }

    return SPI_OK;
}

/**
 * FR-7: Drive CS line LOW to select the target device.
 */
void spi_csEnable(void)
{
    gpio_clearPin(SPI_GPIO_PORT, SPI_PIN_CS);
}

/**
 * FR-8: Drive CS line HIGH to deselect the target device.
 */
void spi_csDisable(void)
{
    gpio_setPin(SPI_GPIO_PORT, SPI_PIN_CS);
}