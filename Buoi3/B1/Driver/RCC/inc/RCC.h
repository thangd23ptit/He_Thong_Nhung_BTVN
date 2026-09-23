#ifndef __RCC_H
#define __RCC_H

#include <stdint.h>

#define RCC_BASE        0x40021000UL

#define RCC_CR          (*(volatile uint32_t *)(RCC_BASE + 0x00UL))
#define RCC_CFGR        (*(volatile uint32_t *)(RCC_BASE + 0x04UL))
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x18UL))
#define RCC_APB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x1CUL))

#define RCC_CR_HSEON    (1U << 16)
#define RCC_CR_HSERDY   (1U << 17)
#define RCC_CR_PLLON    (1U << 24)
#define RCC_CR_PLLRDY   (1U << 25)

#define RCC_APB2_IOPAEN     (1U << 2)
#define RCC_APB2_IOPBEN     (1U << 3)
#define RCC_APB2_AFIOEN     (1U << 0)
#define RCC_APB2_USART1EN   (1U << 14)

#define RCC_APB1_I2C1EN    (1U << 21)

void RCC_Config_72Mhz(void);

void RCC_Enable_PortA(void);
void RCC_Enable_PortB(void);
void RCC_Enable_AFIO(void);
void RCC_Enable_UART1(void);
void RCC_Enable_I2C1(void);

#endif