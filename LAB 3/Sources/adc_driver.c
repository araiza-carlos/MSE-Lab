/**
 * @file    adc_driver.c
 * @brief   ADC Driver Implementation for STM32F4xx
 *
 * Implements all functions defined in adc_driver.h using direct
 * register access via CMSIS peripheral definitions. Supports
 * single and continuous regular conversions, injected channel
 * conversions, and 12-bit result retrieval.
 *
 * PA0 is configured as analog input during adc_init() using
 * direct register writes to avoid conflicts with other modules
 * sharing GPIOA (e.g. the PWM module on PA6).
 *
 * Clock assumption: 16 MHz HSI (default after reset, no PLL).
 *
 * @author  Carlos Humberto Araiza Quintana
 * @version 1.0
 */

#include "adc_driver.h"
#include "stm32f4xx.h"
#include "gpio_driver.h"

/* -----------------------------------------------------------------------
 * Private Definitions
 * --------------------------------------------------------------------- */

#define ADC_MAX_CHANNEL              18U        /**< Highest valid channel (IN0–IN18) */
#define ADC_MAX_INJECTED_CHANNEL      4U        /**< Max injected channels (JDR1–JDR4) */

/* CR2 bit positions */
#define ADC_CR2_ADON_BIT         (1U << 0)     /**< ADC on/off control               */
#define ADC_CR2_CONT_BIT         (1U << 1)     /**< Continuous conversion mode        */
#define ADC_CR2_SWSTART_BIT      (1U << 30)    /**< Start regular conversion          */
#define ADC_CR2_JSWSTART_BIT     (1U << 22)    /**< Start injected conversion         */

/* SR flag positions */
#define ADC_SR_EOC_BIT           (1U << 1)     /**< End of regular conversion         */
#define ADC_SR_JEOC_BIT          (1U << 2)     /**< End of injected conversion        */

/* RCC clock enable bits for ADC */
#define RCC_APB2ENR_ADC1EN_BIT   (1U << 8)     /**< ADC1 peripheral clock enable      */

/* -----------------------------------------------------------------------
 * Private Types
 * --------------------------------------------------------------------- */

/**
 * @brief Maps an ADC_Instance_t enum value to its hardware register
 *        pointer and the corresponding RCC clock enable bit.
 */
typedef struct {
    ADC_TypeDef *reg;      /**< Pointer to ADC peripheral registers */
    uint32_t     rccBit;   /**< RCC_APB2ENR bit to enable the clock */
} ADC_InstanceMap_t;

/* -----------------------------------------------------------------------
 * Private Variables
 * --------------------------------------------------------------------- */

/** Lookup table: ADC instance enum → hardware register + clock bit */
static const ADC_InstanceMap_t adcInstanceMap[] = {
    { ADC1, RCC_APB2ENR_ADC1EN_BIT }    /* ADC_INSTANCE_1 */
};

/* -----------------------------------------------------------------------
 * Private Function Prototypes
 * --------------------------------------------------------------------- */

static ADC_Status_t adc_validateInstance(ADC_Instance_t instance);
static ADC_Status_t adc_validateChannel(uint8_t channel);

/* -----------------------------------------------------------------------
 * Private Function Definitions
 * --------------------------------------------------------------------- */

/**
 * @brief  Validates that the requested ADC instance exists.
 * @param  instance  Instance to validate.
 * @return ADC_OK if valid, ADC_INVALID otherwise.
 */
static ADC_Status_t adc_validateInstance(ADC_Instance_t instance)
{
    if (instance > ADC_INSTANCE_1) { return ADC_INVALID; }
    return ADC_OK;
}

/**
 * @brief  Validates that the requested channel number is in range.
 * @param  channel  Channel number to validate (0–18).
 * @return ADC_OK if valid, ADC_INVALID otherwise.
 */
static ADC_Status_t adc_validateChannel(uint8_t channel)
{
    if (channel > ADC_MAX_CHANNEL) { return ADC_INVALID; }
    return ADC_OK;
}

/* -----------------------------------------------------------------------
 * Public Function Definitions
 * --------------------------------------------------------------------- */

ADC_Status_t adc_init(ADC_Instance_t instance)
{
    if (adc_validateInstance(instance) != ADC_OK) { return ADC_INVALID; }

    /* Enable ADC1 peripheral clock */
    RCC->APB2ENR |= adcInstanceMap[instance].rccBit;

    /* Enable GPIOA clock and configure PA0 as analog input.
     * Direct register write avoids calling gpio_initPort(), which
     * could reset MODER and overwrite PA6 alternate function config. */
    RCC->AHB1ENR  |= RCC_AHB1ENR_GPIOAEN;
    GPIOA->MODER  |= (0x3U << (0U * 2U));   /* bits 1:0 = 0b11 → analog */

    /* Reset ADC control registers to default state */
    ADC_TypeDef *adc = adcInstanceMap[instance].reg;
    adc->CR1 = 0x00000000U;
    adc->CR2 = 0x00000000U;

    return ADC_OK;
}

ADC_Status_t adc_enableAdc(ADC_Instance_t instance)
{
    if (adc_validateInstance(instance) != ADC_OK) { return ADC_INVALID; }

    /* Set ADON bit to power on the ADC */
    adcInstanceMap[instance].reg->CR2 |= ADC_CR2_ADON_BIT;

    return ADC_OK;
}

ADC_Status_t adc_setChannel(ADC_Instance_t instance,
                             uint8_t channel,
                             ADC_SampleTime_t sampleTime)
{
    if (adc_validateInstance(instance) != ADC_OK) { return ADC_INVALID; }
    if (adc_validateChannel(channel)   != ADC_OK) { return ADC_INVALID; }

    ADC_TypeDef *adc = adcInstanceMap[instance].reg;

    /* One conversion in sequence (L = 0 in SQR1) */
    adc->SQR1 = 0x00000000U;

    /* Write channel as first entry in regular sequence */
    adc->SQR3 = (uint32_t)(channel & 0x1FU);

    /* Configure sample time — channels 0-9 in SMPR2, 10-18 in SMPR1 */
    if (channel <= 9U)
    {
        adc->SMPR2 &= ~(0x7U << (channel * 3U));
        adc->SMPR2 |=  ((uint32_t)sampleTime << (channel * 3U));
    }
    else
    {
        uint8_t offset = channel - 10U;
        adc->SMPR1 &= ~(0x7U << (offset * 3U));
        adc->SMPR1 |=  ((uint32_t)sampleTime << (offset * 3U));
    }

    return ADC_OK;
}

ADC_Status_t adc_setInjectedChannel(ADC_Instance_t instance,
                                     uint8_t channel,
                                     ADC_SampleTime_t sampleTime)
{
    if (adc_validateInstance(instance) != ADC_OK) { return ADC_INVALID; }
    if (adc_validateChannel(channel)   != ADC_OK) { return ADC_INVALID; }

    ADC_TypeDef *adc = adcInstanceMap[instance].reg;

    /* One injected conversion (JL = 0), channel in JSQ4 (bits 14:10) */
    adc->JSQR  = 0x00000000U;
    adc->JSQR |= (uint32_t)((channel & 0x1FU) << 10U);

    /* Configure sample time — same register layout as regular channels */
    if (channel <= 9U)
    {
        adc->SMPR2 &= ~(0x7U << (channel * 3U));
        adc->SMPR2 |=  ((uint32_t)sampleTime << (channel * 3U));
    }
    else
    {
        uint8_t offset = channel - 10U;
        adc->SMPR1 &= ~(0x7U << (offset * 3U));
        adc->SMPR1 |=  ((uint32_t)sampleTime << (offset * 3U));
    }

    return ADC_OK;
}

ADC_Status_t adc_startSingleConversion(ADC_Instance_t instance)
{
    if (adc_validateInstance(instance) != ADC_OK) { return ADC_INVALID; }

    ADC_TypeDef *adc = adcInstanceMap[instance].reg;

    /* Disable continuous mode, then trigger a single conversion */
    adc->CR2 &= ~ADC_CR2_CONT_BIT;
    adc->CR2 |=  ADC_CR2_SWSTART_BIT;

    return ADC_OK;
}

ADC_Status_t adc_startContinuousConversion(ADC_Instance_t instance)
{
    if (adc_validateInstance(instance) != ADC_OK) { return ADC_INVALID; }

    ADC_TypeDef *adc = adcInstanceMap[instance].reg;

    /* Enable continuous mode and trigger the first conversion */
    adc->CR2 |= ADC_CR2_CONT_BIT;
    adc->CR2 |= ADC_CR2_SWSTART_BIT;

    return ADC_OK;
}

ADC_Status_t adc_startInjectedConversion(ADC_Instance_t instance)
{
    if (adc_validateInstance(instance) != ADC_OK) { return ADC_INVALID; }

    /* Trigger injected conversion via JSWSTART */
    adcInstanceMap[instance].reg->CR2 |= ADC_CR2_JSWSTART_BIT;

    return ADC_OK;
}

ADC_Status_t adc_readData(ADC_Instance_t instance, uint16_t *data)
{
    if (adc_validateInstance(instance) != ADC_OK) { return ADC_INVALID; }
    if (data == NULL)                              { return ADC_ERROR;   }

    ADC_TypeDef *adc = adcInstanceMap[instance].reg;

    /* Wait for End Of Conversion flag */
    while (!(adc->SR & ADC_SR_EOC_BIT));

    /* Read 12-bit result from data register */
    *data = (uint16_t)(adc->DR & 0x0FFFU);

    return ADC_OK;
}

ADC_Status_t adc_readInjectedChannelData(ADC_Instance_t instance,
                                          uint8_t channel,
                                          uint16_t *data)
{
    if (adc_validateInstance(instance) != ADC_OK)            { return ADC_INVALID; }
    if (channel == 0U || channel > ADC_MAX_INJECTED_CHANNEL) { return ADC_INVALID; }
    if (data == NULL)                                        { return ADC_ERROR;   }

    ADC_TypeDef *adc = adcInstanceMap[instance].reg;

    /* Wait for End Of Injected Conversion flag */
    while (!(adc->SR & ADC_SR_JEOC_BIT));

    /* Read from the appropriate injected data register */
    switch (channel)
    {
        case 1U: *data = (uint16_t)(adc->JDR1 & 0x0FFFU); break;
        case 2U: *data = (uint16_t)(adc->JDR2 & 0x0FFFU); break;
        case 3U: *data = (uint16_t)(adc->JDR3 & 0x0FFFU); break;
        case 4U: *data = (uint16_t)(adc->JDR4 & 0x0FFFU); break;
        default: return ADC_ERROR;
    }

    return ADC_OK;
}
