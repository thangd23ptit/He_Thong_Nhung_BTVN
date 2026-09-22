#ifndef __RCC_H
#define __RCC_H

#include <stdint.h>

#define RCC_BASE 0x40021000UL

#define RCC_CR       (*(volatile uint32_t *)(RCC_BASE + 0x00UL))
#define RCC_CFGR     (*(volatile uint32_t *)(RCC_BASE + 0x04UL))
#define RCC_CIR      (*(volatile uint32_t *)(RCC_BASE + 0x08UL))
#define RCC_APB2RSTR (*(volatile uint32_t *)(RCC_BASE + 0x0CUL))
#define RCC_APB1RSTR (*(volatile uint32_t *)(RCC_BASE + 0x10UL))
#define RCC_AHBENR   (*(volatile uint32_t *)(RCC_BASE + 0x14UL))
#define RCC_APB2ENR  (*(volatile uint32_t *)(RCC_BASE + 0x18UL))
#define RCC_APB1ENR  (*(volatile uint32_t *)(RCC_BASE + 0x1CUL))
#define RCC_BDCR     (*(volatile uint32_t *)(RCC_BASE + 0x20UL))
#define RCC_CSR      (*(volatile uint32_t *)(RCC_BASE + 0x24UL))

#define FLASH_ACR (*(volatile uint32_t *)0x40022000UL)

void RCC_Config_72Mhz(void);

void RCC_Enable_DMA1(void);

void RCC_Enable_AFIO(void);

void RCC_Enable_PortA(void);
void RCC_Enable_PortB(void);
void RCC_Enable_PortC(void);

void RCC_Enable_ADC1(void);
void RCC_Enable_ADC2(void);

void RCC_Enable_TIM2(void);
void RCC_Enable_TIM3(void);
void RCC_Enable_TIM4(void);

void RCC_Enable_UART1(void);
void RCC_Enable_UART2(void);
void RCC_Enable_UART3(void);

void RCC_Enable_SPI1(void);
void RCC_Enable_SPI2(void);

void RCC_Enable_I2C1(void);
void RCC_Enable_I2C2(void);

void RCC_Enable_USB(void);

#endif