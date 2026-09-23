#ifndef __GPIO_H
#define __GPIO_H

#include <stdint.h>

#define GPIOA_BASE      0x40010800UL
#define GPIOB_BASE      0x40010C00UL
#define GPIOC_BASE      0x40011000UL

#define GPIOA_CRL       (*(volatile uint32_t *)(GPIOA_BASE + 0x00UL))
#define GPIOA_CRH       (*(volatile uint32_t *)(GPIOA_BASE + 0x04UL))
#define GPIOA_IDR       (*(volatile uint32_t *)(GPIOA_BASE + 0x08UL))
#define GPIOA_ODR       (*(volatile uint32_t *)(GPIOA_BASE + 0x0CUL))

#define GPIOB_CRL       (*(volatile uint32_t *)(GPIOB_BASE + 0x00UL))
#define GPIOB_CRH       (*(volatile uint32_t *)(GPIOB_BASE + 0x04UL))
#define GPIOB_IDR       (*(volatile uint32_t *)(GPIOB_BASE + 0x08UL))
#define GPIOB_ODR       (*(volatile uint32_t *)(GPIOB_BASE + 0x0CUL))

#define GPIOC_CRL       (*(volatile uint32_t *)(GPIOC_BASE + 0x00UL))
#define GPIOC_CRH       (*(volatile uint32_t *)(GPIOC_BASE + 0x04UL))
#define GPIOC_IDR       (*(volatile uint32_t *)(GPIOC_BASE + 0x08UL))
#define GPIOC_ODR       (*(volatile uint32_t *)(GPIOC_BASE + 0x0CUL))

#define GPIO_PIN_0      ((uint16_t)0x0001)
#define GPIO_PIN_1      ((uint16_t)0x0002)
#define GPIO_PIN_2      ((uint16_t)0x0004)
#define GPIO_PIN_3      ((uint16_t)0x0008)
#define GPIO_PIN_4      ((uint16_t)0x0010)
#define GPIO_PIN_5      ((uint16_t)0x0020)
#define GPIO_PIN_6      ((uint16_t)0x0040)
#define GPIO_PIN_7      ((uint16_t)0x0080)
#define GPIO_PIN_8      ((uint16_t)0x0100)
#define GPIO_PIN_9      ((uint16_t)0x0200)
#define GPIO_PIN_10     ((uint16_t)0x0400)
#define GPIO_PIN_11     ((uint16_t)0x0800)
#define GPIO_PIN_12     ((uint16_t)0x1000)
#define GPIO_PIN_13     ((uint16_t)0x2000)
#define GPIO_PIN_14     ((uint16_t)0x4000)
#define GPIO_PIN_15     ((uint16_t)0x8000)

#define GPIO_MODE_INPUT_FLOATING   0
#define GPIO_MODE_OUTPUT_PP        1
#define GPIO_MODE_OUTPUT_OD        2
#define GPIO_MODE_AF_PP            3
#define GPIO_MODE_AF_OD            4
#define GPIO_MODE_INPUT_ANALOG     5
#define GPIO_MODE_INPUT_PU         6
#define GPIO_MODE_INPUT_PD         7

typedef enum
{
    GPIO_PORT_A = 0,
    GPIO_PORT_B,
    GPIO_PORT_C
} GPIO_Port;

void GPIO_Config(
    GPIO_Port port,
    uint16_t pin,
    uint32_t mode
);

uint8_t GPIO_Read_Pin(
    GPIO_Port port,
    uint16_t pin
);

void GPIO_Write_Pin(
    GPIO_Port port,
    uint16_t pin,
    uint8_t state
);

#endif