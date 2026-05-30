/******************************************************************************
 * Copyright (C) 2026 by Carlos Villarreal - CETYS Universidad
 *****************************************************************************/
/**
 * @file    i2c.c
 * @brief   I2C1 bare-metal driver for STM32F411RE
 *
 * Implements the full API defined in i2c.h (SRS v1.0):
 *   i2c_init, i2c_writeRegDevice, i2c_writeDevice,
 *   i2c_readRegDevice, i2c_readDevice
 *
 * Hardware notes:
 *   - PB8 (SCL) y PB9 (SDA) como AF4, open-drain.
 *   - Se requieren resistencias externas de 4.7 kΩ a 3.3 V.
 *     Los pull-ups internos (~40 kΩ) son demasiado débiles para 100 kHz.
 *   - 100 kHz @ APB1 = 16 MHz: CR2=16, CCR=80, TRISE=17.
 *
 * @author  Vania Leal
 * @date    05/15/2026
 *
 * @ref     IEEE Std 830-1998
 * @ref     RM0383 STM32F411xC/E Reference Manual §18
 */

#include "i2c.h"
#include "gpio_driver.h"
 
/* ============================================================
 * Internal helpers (low-level primitives)
 * ============================================================ */
 
/** Generate START and wait for SB flag. */
static i2c_status_t _i2c_start(void)
{
    uint32_t timeout = I2C_TIMEOUT;
 
    /* Wait until bus is free (BUSY flag in SR2) */
    while (I2C1->SR2 & I2C_SR2_BUSY)
    {
        if (--timeout == 0U) return I2C_BUSY;
    }
 
    I2C1->CR1 |= I2C_CR1_START;
 
    timeout = I2C_TIMEOUT;
    while (!(I2C1->SR1 & I2C_SR1_SB))
    {
        if (--timeout == 0U)
        {
            I2C1->CR1 |= I2C_CR1_STOP;
            return I2C_TIMEOUT_ERROR;
        }
    }
 
    return I2C_OK;
}
 
/** Generate STOP condition. */
static void _i2c_stop(void)
{
    I2C1->CR1 |= I2C_CR1_STOP;
}
 
/**
 * Send 7-bit address with R/W bit and wait for ADDR flag.
 * Clears the AF flag on failure to avoid corrupting next transaction.
 */
static i2c_status_t _i2c_sendAddress(uint8_t address, uint8_t read)
{
    uint32_t timeout = I2C_TIMEOUT;
 
    uint8_t addr = (uint8_t)(address << 1U);
    if (read) addr |= 1U;
 
    (void)I2C1->SR1;   /* Clear SB by reading SR1 before writing DR */
    I2C1->DR = addr;
 
    while (!(I2C1->SR1 & I2C_SR1_ADDR))
    {
        if (--timeout == 0U)
        {
            I2C1->SR1 &= ~I2C_SR1_AF;   /* Clear AF to unblock next transaction */
            I2C1->CR1 |= I2C_CR1_STOP;
            return I2C_TIMEOUT_ERROR;
        }
    }
 
    /* Clear ADDR flag: read SR1 then SR2 */
    (void)I2C1->SR1;
    (void)I2C1->SR2;
 
    return I2C_OK;
}
 
/** Transmit one byte and wait for BTF. */
static i2c_status_t _i2c_writeByte(uint8_t data)
{
    uint32_t timeout = I2C_TIMEOUT;
 
    while (!(I2C1->SR1 & I2C_SR1_TXE))
    {
        if (--timeout == 0U) return I2C_TIMEOUT_ERROR;
    }
 
    I2C1->DR = data;
 
    timeout = I2C_TIMEOUT;
    while (!(I2C1->SR1 & I2C_SR1_BTF))
    {
        if (--timeout == 0U) return I2C_TIMEOUT_ERROR;
    }
 
    return I2C_OK;
}
 
/**
 * Receive one byte.
 * @param ack  1 = send ACK (more bytes follow), 0 = send NACK (last byte).
 */
static i2c_status_t _i2c_readByte(uint8_t *data, uint8_t ack)
{
    uint32_t timeout = I2C_TIMEOUT;
 
    if (ack)
        I2C1->CR1 |= I2C_CR1_ACK;
    else
        I2C1->CR1 &= ~I2C_CR1_ACK;
 
    while (!(I2C1->SR1 & I2C_SR1_RXNE))
    {
        if (--timeout == 0U) return I2C_TIMEOUT_ERROR;
    }
 
    *data = (uint8_t)I2C1->DR;
 
    return I2C_OK;
}
 
/* ============================================================
 * FR-1, FR-2 — i2c_init
 * ============================================================ */
 
i2c_status_t i2c_init(void)
{
    /* Enable GPIOB clock and configure PB8/PB9 as AF4, open-drain */
    gpio_initPort(GPIOB);
    gpio_setAlternateFunction(GPIOB, GPIO_PIN_8, 4U);
    gpio_setAlternateFunction(GPIOB, GPIO_PIN_9, 4U);
 
    GPIOB->OTYPER  |=  (1U << 8U) | (1U << 9U);           /* Open-drain      */
    GPIOB->PUPDR   &= ~((3U << 16U) | (3U << 18U));        /* No internal PU  */
    GPIOB->OSPEEDR |=  (3U << 16U)  | (3U << 18U);         /* High speed      */
 
    /* Enable I2C1 clock (APB1) */
    RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
 
    /* Software reset to clear any previous state */
    I2C1->CR1 |=  I2C_CR1_SWRST;
    I2C1->CR1 &= ~I2C_CR1_SWRST;
 
    /*
     * Standard mode 100 kHz @ 16 MHz APB1  (RM0383 §18.3.3)
     *   CR2  FREQ  = 16  (APB1 in MHz)
     *   CCR         = 80  (fPCLK / (2 × fSCL) = 16e6 / 200e3)
     *   TRISE       = 17  (1000 ns / (1/16 MHz) + 1)
     */
    I2C1->CR2   = 16U;
    I2C1->CCR   = 80U;
    I2C1->TRISE = 17U;
 
    /* Enable peripheral */
    I2C1->CR1 |= I2C_CR1_PE;
 
    return I2C_OK;
}
 
/* ============================================================
 * FR-3, FR-4 — i2c_writeRegDevice
 * ============================================================ */
 
i2c_status_t i2c_writeRegDevice(uint8_t dev_addr, uint8_t reg_addr,
                                 const uint8_t *data, size_t len)
{
    if (data == NULL) return I2C_INVALID;
 
    i2c_status_t st;
 
    st = _i2c_start();                          if (st != I2C_OK) return st;
    st = _i2c_sendAddress(dev_addr, 0U);        if (st != I2C_OK) { _i2c_stop(); return st; }
    st = _i2c_writeByte(reg_addr);              if (st != I2C_OK) { _i2c_stop(); return st; }
 
    for (size_t i = 0U; i < len; i++)
    {
        st = _i2c_writeByte(data[i]);
        if (st != I2C_OK) { _i2c_stop(); return st; }
    }
 
    _i2c_stop();
    return I2C_OK;
}
 
/* ============================================================
 * FR-5, FR-6 — i2c_writeDevice
 * ============================================================ */
 
i2c_status_t i2c_writeDevice(uint8_t dev_addr, const uint8_t *data, size_t len)
{
    if (data == NULL) return I2C_INVALID;
 
    i2c_status_t st;
 
    st = _i2c_start();                          if (st != I2C_OK) return st;
    st = _i2c_sendAddress(dev_addr, 0U);        if (st != I2C_OK) { _i2c_stop(); return st; }
 
    for (size_t i = 0U; i < len; i++)
    {
        st = _i2c_writeByte(data[i]);
        if (st != I2C_OK) { _i2c_stop(); return st; }
    }
 
    _i2c_stop();
    return I2C_OK;
}
 
/* ============================================================
 * FR-7, FR-8 — i2c_readRegDevice
 * ============================================================ */
 
i2c_status_t i2c_readRegDevice(uint8_t dev_addr, uint8_t reg_addr,
                                uint8_t *buf, size_t len)
{
    if (buf == NULL || len == 0U) return I2C_INVALID;
 
    i2c_status_t st;
 
    /* Phase 1: write register address */
    st = _i2c_start();                          if (st != I2C_OK) return st;
    st = _i2c_sendAddress(dev_addr, 0U);        if (st != I2C_OK) { _i2c_stop(); return st; }
    st = _i2c_writeByte(reg_addr);              if (st != I2C_OK) { _i2c_stop(); return st; }
 
    /* Phase 2: repeated START then read */
    st = _i2c_start();                          if (st != I2C_OK) return st;
    st = _i2c_sendAddress(dev_addr, 1U);        if (st != I2C_OK) { _i2c_stop(); return st; }
 
    for (size_t i = 0U; i < len; i++)
    {
        uint8_t ack = (i < (len - 1U)) ? 1U : 0U;   /* NACK on last byte */
        st = _i2c_readByte(&buf[i], ack);
        if (st != I2C_OK) { _i2c_stop(); return st; }
    }
 
    _i2c_stop();
    return I2C_OK;
}
 
/* ============================================================
 * FR-9, FR-10 — i2c_readDevice
 * ============================================================ */
 
i2c_status_t i2c_readDevice(uint8_t dev_addr, uint8_t *buf, size_t len)
{
    if (buf == NULL || len == 0U) return I2C_INVALID;
 
    i2c_status_t st;
 
    st = _i2c_start();                          if (st != I2C_OK) return st;
    st = _i2c_sendAddress(dev_addr, 1U);        if (st != I2C_OK) { _i2c_stop(); return st; }
 
    for (size_t i = 0U; i < len; i++)
    {
        uint8_t ack = (i < (len - 1U)) ? 1U : 0U;
        st = _i2c_readByte(&buf[i], ack);
        if (st != I2C_OK) { _i2c_stop(); return st; }
    }
 
    _i2c_stop();
    return I2C_OK;
}
 