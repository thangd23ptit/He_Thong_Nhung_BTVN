#ifndef STM32F10X_H
#define STM32F10X_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

static inline void __NOP(void)
{
    __asm volatile ("nop");
}

/*
 * Minimal CMSIS-style register definitions for STM32F103x8/xB
 * (STM32F1 medium-density), suitable for bare-metal code.
 *
 * Main peripherals covered:
 *   - RCC
 *   - GPIOA/B/C/D
 *   - USART1/2/3
 *   - SysTick (ARM Cortex-M3 core peripheral)
 *
 * This header is intended to fix errors such as:
 *   RCC undeclared
 *   GPIOA undeclared
 *   USART1 undeclared
 *   SysTick undeclared
 */

/* -------------------------------------------------------------------------- */
/* Common types                                                               */
/* -------------------------------------------------------------------------- */

#ifndef __IO
#define __IO volatile
#endif

#ifndef __I
#define __I  volatile const
#endif

#ifndef __O
#define __O  volatile
#endif

/* -------------------------------------------------------------------------- */
/* Peripheral structures                                                      */
/* -------------------------------------------------------------------------- */

typedef struct
{
    __IO uint32_t CR;        /* 0x00 */
    __IO uint32_t CFGR;      /* 0x04 */
    __IO uint32_t CIR;       /* 0x08 */
    __IO uint32_t APB2RSTR;  /* 0x0C */
    __IO uint32_t APB1RSTR;  /* 0x10 */
    __IO uint32_t AHBENR;    /* 0x14 */
    __IO uint32_t APB2ENR;   /* 0x18 */
    __IO uint32_t APB1ENR;   /* 0x1C */
    __IO uint32_t BDCR;      /* 0x20 */
    __IO uint32_t CSR;       /* 0x24 */
    __IO uint32_t AHBSTR;    /* 0x28 */
    __IO uint32_t CFGR2;     /* 0x2C */
} RCC_TypeDef;

typedef struct
{
    __IO uint32_t CRL;       /* 0x00 */
    __IO uint32_t CRH;       /* 0x04 */
    __IO uint32_t IDR;       /* 0x08 */
    __IO uint32_t ODR;       /* 0x0C */
    __IO uint32_t BSRR;      /* 0x10 */
    __IO uint32_t BRR;       /* 0x14 */
    __IO uint32_t LCKR;      /* 0x18 */
} GPIO_TypeDef;

typedef struct
{
    __IO uint32_t SR;        /* 0x00 */
    __IO uint32_t DR;        /* 0x04 */
    __IO uint32_t BRR;       /* 0x08 */
    __IO uint32_t CR1;       /* 0x0C */
    __IO uint32_t CR2;       /* 0x10 */
    __IO uint32_t CR3;       /* 0x14 */
    __IO uint32_t GTPR;      /* 0x18 */
} USART_TypeDef;

typedef struct
{
    __IO uint32_t CR1;       /* 0x00 */
    __IO uint32_t CR2;       /* 0x04 */
    __IO uint32_t OAR1;      /* 0x08 */
    __IO uint32_t OAR2;      /* 0x0C */
    __IO uint32_t DR;        /* 0x10 */
    __IO uint32_t SR1;       /* 0x14 */
    __IO uint32_t SR2;       /* 0x18 */
    __IO uint32_t CCR;       /* 0x1C */
    __IO uint32_t TRISE;     /* 0x20 */
    __IO uint32_t FLTR;      /* 0x24 */
} I2C_TypeDef;

typedef struct
{
    __IO uint32_t CCR;       /* 0x00 */
    __IO uint32_t CNDTR;     /* 0x04 */
    __IO uint32_t CPAR;      /* 0x08 */
    __IO uint32_t CMAR;      /* 0x0C */
} DMA_Channel_TypeDef;

typedef struct
{
    __IO uint32_t ISR;       /* 0x00 */
    __IO uint32_t IFCR;      /* 0x04 */
} DMA_TypeDef;

typedef struct
{
    __IO uint32_t CR1;       /* 0x00 */
    __IO uint32_t CR2;       /* 0x04 */
    __IO uint32_t SR;        /* 0x08 */
    __IO uint32_t DR;        /* 0x0C */
    __IO uint32_t CRCPR;     /* 0x10 */
    __IO uint32_t RXCRCR;    /* 0x14 */
    __IO uint32_t TXCRCR;    /* 0x18 */
    __IO uint32_t I2SCFGR;   /* 0x1C */
    __IO uint32_t I2SPR;     /* 0x20 */
} SPI_TypeDef;

/* SysTick - ARM Cortex-M3 core peripheral (không phụ thuộc hãng chip) */
typedef struct
{
    __IO uint32_t CTRL;      /* 0x00: Control and Status Register */
    __IO uint32_t LOAD;      /* 0x04: Reload Value Register */
    __IO uint32_t VAL;       /* 0x08: Current Value Register */
    __I  uint32_t CALIB;     /* 0x0C: Calibration Register */
} SysTick_TypeDef;


/* ADC1 register map (STM32F103) */
typedef struct {
    volatile uint32_t SR;
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t SMPR1;
    volatile uint32_t SMPR2;
    volatile uint32_t JOFR1;
    volatile uint32_t JOFR2;
    volatile uint32_t JOFR3;
    volatile uint32_t JOFR4;
    volatile uint32_t HTR;
    volatile uint32_t LTR;
    volatile uint32_t SQR1;
    volatile uint32_t SQR2;
    volatile uint32_t SQR3;
    volatile uint32_t JSQR;
    volatile uint32_t JDR1;
    volatile uint32_t JDR2;
    volatile uint32_t JDR3;
    volatile uint32_t JDR4;
    volatile uint32_t DR;
} ADC_TypeDef;

#define ADC1_BASE   0x40012400UL
#define ADC1        ((ADC_TypeDef *)ADC1_BASE)

/* FLASH register map (STM32F103) - Flash Access Control Register */
typedef struct
{
    __IO uint32_t ACR;       /* 0x00: Access control register */
    __IO uint32_t KEYR;      /* 0x04: FPEC key register */
    __IO uint32_t OPTKEYR;   /* 0x08: Option byte key register */
    __IO uint32_t SR;        /* 0x0C: Status register */
    __IO uint32_t CR;        /* 0x10: Control register */
    __IO uint32_t AR;        /* 0x14: Address register */
    __IO uint32_t RESERVED;  /* 0x18: Reserved */
    __IO uint32_t OBR;       /* 0x1C: Option byte register */
    __IO uint32_t WRPR;      /* 0x20: Write protection register */
} FLASH_TypeDef;
/* -------------------------------------------------------------------------- */
/* Peripheral base addresses                                                   */
/* -------------------------------------------------------------------------- */

#define PERIPH_BASE           0x40000000UL
#define APB1PERIPH_BASE       PERIPH_BASE
#define APB2PERIPH_BASE       0x40010000UL
#define AHBPERIPH_BASE        0x40018000UL

/* RCC */
#define RCC_BASE              0x40021000UL
/* FLASH */
#define FLASH_BASE            0x40022000UL

/* GPIO */
#define GPIOA_BASE            (APB2PERIPH_BASE + 0x0800UL)
#define GPIOB_BASE            (APB2PERIPH_BASE + 0x0C00UL)
#define GPIOC_BASE            (APB2PERIPH_BASE + 0x1000UL)
#define GPIOD_BASE            (APB2PERIPH_BASE + 0x1400UL)

/* SPI */
#define SPI1_BASE             (APB2PERIPH_BASE + 0x3000UL)
#define SPI2_BASE             (APB1PERIPH_BASE + 0x3800UL)

/* USART */
#define USART1_BASE           (APB2PERIPH_BASE + 0x3800UL)
#define USART2_BASE           (APB1PERIPH_BASE + 0x4400UL)
#define USART3_BASE           (APB1PERIPH_BASE + 0x4800UL)

/* I2C */
#define I2C1_BASE             (APB1PERIPH_BASE + 0x5400UL)
#define I2C2_BASE             (APB1PERIPH_BASE + 0x5800UL)

/* DMA1 */
/* Theo RM0008: AHBPERIPH_BASE (0x4001_8000) là base của SDIO, KHÔNG PHẢI DMA1.
 * DMA1 thực sự nằm ở 0x4002_0000 (ngay trước RCC 0x4002_1000). */
#define DMA1_BASE             0x40020000UL
#define DMA1_Channel4_BASE    (DMA1_BASE + 0x0044UL)

/* SysTick - vùng địa chỉ System Control Space (SCS) của Cortex-M3 */
#define SCS_BASE              0xE000E000UL
#define SysTick_BASE          (SCS_BASE + 0x0010UL)   /* = 0xE000E010 */

/* -------------------------------------------------------------------------- */
/* Peripheral instances                                                       */
/* -------------------------------------------------------------------------- */

#define RCC                 ((RCC_TypeDef   *)RCC_BASE)
#define FLASH               ((FLASH_TypeDef *)FLASH_BASE)

#define GPIOA               ((GPIO_TypeDef  *)GPIOA_BASE)
#define GPIOB               ((GPIO_TypeDef  *)GPIOB_BASE)
#define GPIOC               ((GPIO_TypeDef  *)GPIOC_BASE)
#define GPIOD               ((GPIO_TypeDef  *)GPIOD_BASE)

#define SPI1                ((SPI_TypeDef *)SPI1_BASE)
#define SPI2                ((SPI_TypeDef *)SPI2_BASE)

#define USART1              ((USART_TypeDef *)USART1_BASE)
#define USART2              ((USART_TypeDef *)USART2_BASE)
#define USART3              ((USART_TypeDef *)USART3_BASE)

#define I2C1                ((I2C_TypeDef *)I2C1_BASE)
#define I2C2                ((I2C_TypeDef *)I2C2_BASE)

#define DMA1                ((DMA_TypeDef *)DMA1_BASE)
#define DMA1_Channel4       ((DMA_Channel_TypeDef *)DMA1_Channel4_BASE)

#define SysTick             ((SysTick_TypeDef *)SysTick_BASE)

/* -------------------------------------------------------------------------- */
/* RCC APB2 peripheral clock enable bits                                      */
/* -------------------------------------------------------------------------- */

#define RCC_APB2ENR_AFIOEN_Pos       0U
#define RCC_APB2ENR_IOPAEN_Pos       2U
#define RCC_APB2ENR_IOPBEN_Pos       3U
#define RCC_APB2ENR_IOPCEN_Pos       4U
#define RCC_APB2ENR_IOPDEN_Pos       5U
#define RCC_APB2ENR_USART1EN_Pos    14U

#define RCC_APB2ENR_AFIOEN           (1UL << RCC_APB2ENR_AFIOEN_Pos)
#define RCC_APB2ENR_IOPAEN           (1UL << RCC_APB2ENR_IOPAEN_Pos)
#define RCC_APB2ENR_IOPBEN           (1UL << RCC_APB2ENR_IOPBEN_Pos)
#define RCC_APB2ENR_IOPCEN           (1UL << RCC_APB2ENR_IOPCEN_Pos)
#define RCC_APB2ENR_IOPDEN           (1UL << RCC_APB2ENR_IOPDEN_Pos)
#define RCC_APB2ENR_USART1EN         (1UL << RCC_APB2ENR_USART1EN_Pos)

/* -------------------------------------------------------------------------- */
/* RCC APB1 peripheral clock enable bits                                      */
/* -------------------------------------------------------------------------- */

#define RCC_APB1ENR_USART2EN_Pos     17U
#define RCC_APB1ENR_USART3EN_Pos     18U

#define RCC_APB1ENR_USART2EN         (1UL << RCC_APB1ENR_USART2EN_Pos)
#define RCC_APB1ENR_USART3EN         (1UL << RCC_APB1ENR_USART3EN_Pos)

/* -------------------------------------------------------------------------- */
/* GPIO CRL/CRH helper bit definitions                                        */
/* -------------------------------------------------------------------------- */

#define GPIO_CRL_MODE0_Pos           0U
#define GPIO_CRL_CNF0_Pos            2U

#define GPIO_CRH_MODE8_Pos           0U
#define GPIO_CRH_CNF8_Pos            2U

/* -------------------------------------------------------------------------- */
/* USART status register bits                                                 */
/* -------------------------------------------------------------------------- */

#define USART_SR_PE_Pos              0U
#define USART_SR_FE_Pos              1U
#define USART_SR_NE_Pos              2U
#define USART_SR_ORE_Pos             3U
#define USART_SR_IDLE_Pos            4U
#define USART_SR_RXNE_Pos            5U
#define USART_SR_TC_Pos              6U
#define USART_SR_TXE_Pos             7U

#define USART_SR_PE                  (1UL << USART_SR_PE_Pos)
#define USART_SR_FE                  (1UL << USART_SR_FE_Pos)
#define USART_SR_NE                  (1UL << USART_SR_NE_Pos)
#define USART_SR_ORE                 (1UL << USART_SR_ORE_Pos)
#define USART_SR_IDLE                (1UL << USART_SR_IDLE_Pos)
#define USART_SR_RXNE                (1UL << USART_SR_RXNE_Pos)
#define USART_SR_TC                  (1UL << USART_SR_TC_Pos)
#define USART_SR_TXE                 (1UL << USART_SR_TXE_Pos)

/* -------------------------------------------------------------------------- */
/* USART CR1 bits                                                             */
/* -------------------------------------------------------------------------- */

#define USART_CR1_SBK_Pos            0U
#define USART_CR1_RWU_Pos            1U
#define USART_CR1_RE_Pos             2U
#define USART_CR1_TE_Pos             3U
#define USART_CR1_IDLEIE_Pos         4U
#define USART_CR1_RXNEIE_Pos         5U
#define USART_CR1_TCIE_Pos           6U
#define USART_CR1_TXEIE_Pos          7U
#define USART_CR1_PEIE_Pos           8U
#define USART_CR1_PS_Pos             9U
#define USART_CR1_PCE_Pos           10U
#define USART_CR1_WAKE_Pos          11U
#define USART_CR1_M_Pos             12U
#define USART_CR1_UE_Pos            13U

#define USART_CR1_RE                 (1UL << USART_CR1_RE_Pos)
#define USART_CR1_TE                 (1UL << USART_CR1_TE_Pos)
#define USART_CR1_RXNEIE             (1UL << USART_CR1_RXNEIE_Pos)
#define USART_CR1_TCIE               (1UL << USART_CR1_TCIE_Pos)
#define USART_CR1_TXEIE              (1UL << USART_CR1_TXEIE_Pos)
#define USART_CR1_PS                 (1UL << USART_CR1_PS_Pos)
#define USART_CR1_PCE                (1UL << USART_CR1_PCE_Pos)
#define USART_CR1_M                 (1UL << USART_CR1_M_Pos)
#define USART_CR1_UE                (1UL << USART_CR1_UE_Pos)

/* -------------------------------------------------------------------------- */
/* USART CR2 bits                                                             */
/* -------------------------------------------------------------------------- */

#define USART_CR2_STOP_Pos           12U
#define USART_CR2_STOP_Msk           (3UL << USART_CR2_STOP_Pos)

/* -------------------------------------------------------------------------- */
/* USART CR3 bits                                                             */
/* -------------------------------------------------------------------------- */

#define USART_CR3_EIE_Pos             0U
#define USART_CR3_DMAR_Pos            6U
#define USART_CR3_DMAT_Pos            7U

#define USART_CR3_EIE                 (1UL << USART_CR3_EIE_Pos)
#define USART_CR3_DMAR                (1UL << USART_CR3_DMAR_Pos)
#define USART_CR3_DMAT                (1UL << USART_CR3_DMAT_Pos)

/* -------------------------------------------------------------------------- */
/* SysTick control/status bits (SysTick->CTRL)                                */
/* -------------------------------------------------------------------------- */

#define SysTick_CTRL_ENABLE_Pos       0U
#define SysTick_CTRL_TICKINT_Pos      1U
#define SysTick_CTRL_CLKSOURCE_Pos    2U
#define SysTick_CTRL_COUNTFLAG_Pos   16U

#define SysTick_CTRL_ENABLE          (1UL << SysTick_CTRL_ENABLE_Pos)
#define SysTick_CTRL_TICKINT         (1UL << SysTick_CTRL_TICKINT_Pos)
#define SysTick_CTRL_CLKSOURCE       (1UL << SysTick_CTRL_CLKSOURCE_Pos) /* 1 = processor clock (AHB), 0 = AHB/8 */
#define SysTick_CTRL_COUNTFLAG       (1UL << SysTick_CTRL_COUNTFLAG_Pos)

/* SysTick->LOAD chỉ có 24 bit hợp lệ (bit 24-31 dự trữ) */
#define SysTick_LOAD_RELOAD_Msk      (0x00FFFFFFUL)

/* -------------------------------------------------------------------------- */
/* Common device macros                                                       */
/* -------------------------------------------------------------------------- */

#define SET_BIT(REG, BIT)             ((REG) |= (BIT))
#define CLEAR_BIT(REG, BIT)           ((REG) &= ~(BIT))
#define READ_BIT(REG, BIT)            ((REG) & (BIT))
#define CLEAR_REG(REG)                ((REG) = 0UL)
#define WRITE_REG(REG, VAL)           ((REG) = (VAL))
#define READ_REG(REG)                 (REG)

#ifdef __cplusplus
}
#endif

#endif /* STM32F10X_H */