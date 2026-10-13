#include "stm32f10x.h"
#include "uart.h"
#include "timer.h"
#include "adc.h"
#include "dma.h"

/* ---------------- Clock 72MHz (giu nguyen tu bai truoc) ---------------- */
void SystemClock_Config(void)
{
    RCC->CR |= (1 << 16);                       // HSEON
    while (!(RCC->CR & (1 << 17)));             // HSERDY

    FLASH->ACR |= (1 << 4);                     // Prefetch
    FLASH->ACR &= ~(0x7);
    FLASH->ACR |= (0x2);                        // 2 wait state

    RCC->CFGR &= ~(0xF << 4);                   // AHB  = /1
    RCC->CFGR &= ~(0x7 << 8);
    RCC->CFGR |=  (0x4 << 8);                   // APB1 = /2 (36MHz)
    RCC->CFGR &= ~(0x7 << 11);                  // APB2 = /1 (72MHz)

    RCC->CFGR |=  (1 << 16);                    // PLLSRC = HSE
    RCC->CFGR &= ~(1 << 17);                    // PLLXTPRE = 0
    RCC->CFGR &= ~(0xF << 18);
    RCC->CFGR |=  (0x7 << 18);                  // x9 -> 72MHz

    RCC->CR |= (1 << 24);                       // PLLON
    while (!(RCC->CR & (1 << 25)));             // PLLRDY

    RCC->CFGR &= ~(0x3);
    RCC->CFGR |=  (0x2);                        // SW = PLL
    while (((RCC->CFGR >> 2) & 0x3) != 0x2);
}

/* LED tren Blue Pill: PC13 (muc thap = sang). Nhay moi lan co ngat HT/TC */
static void Led_Init(void)
{
    RCC->APB2ENR |= (1 << 4);                   // IOPCEN
    GPIOC->CRH &= ~(0xF << 20);
    GPIOC->CRH |=  (0x2 << 20);                 // Output 2MHz, push-pull
    GPIOC->ODR |=  (1 << 13);                   // tat LED
}
static void Led_Toggle(void) { GPIOC->ODR ^= (1 << 13); }

/* ---------------- Doi so -> chuoi, tu them "\n\r" sau moi so ---------------- */
static uint16_t Format_Samples(uint8_t *dst, volatile uint16_t *src, uint16_t n)
{
    uint16_t len = 0;
    for (uint16_t i = 0; i < n; i++)
    {
        uint16_t v = src[i];
        uint8_t  tmp[5];
        uint8_t  cnt = 0;

        if (v == 0) tmp[cnt++] = '0';
        while (v) { tmp[cnt++] = '0' + (v % 10); v /= 10; }
        while (cnt) dst[len++] = tmp[--cnt];

        dst[len++] = '\n';
        dst[len++] = '\r';
    }
    return len;
}

/* 50 mau x toi da 6 byte ("4095\n\r") = 300 byte */
static uint8_t tx_buf[ADC_HALF_SIZE * 6];

static void Send_Half(volatile uint16_t *half)
{
    /* Doi DMA UART gui xong dot truoc (khong ghi de tx_buf) */
    while (DMA1_Channel4->CNDTR > 0);

    uint16_t len = Format_Samples(tx_buf, half, ADC_HALF_SIZE);
    UART1_Send_DMA(tx_buf, len);
}

int main(void)
{
    SystemClock_Config();
    Led_Init();

    TIM3_Init_100Hz();          // Timer 100Hz, TRGO = update
    UART1_DMA_Init();           // UART1 115200 + DMA1 Ch4 (TX)
    ADC_DMA_Init();             // DMA1 Ch1: ADC1->DR -> adc_buf, HT/TC interrupt
    ADC1_Init_TimerTrigger();   // ADC1 CH5 (PA5), trigger TIM3_TRGO, DMA
    TIM3_Start();               // Bat dau: 100 mau/giay

    while (1)
    {
        if (dma_half_flag)      // Nua dau [0..49] an toan
        {
            dma_half_flag = 0;
            Send_Half(&adc_buf[0]);
            Led_Toggle();
        }
        if (dma_full_flag)      // Nua sau [50..99] an toan
        {
            dma_full_flag = 0;
            Send_Half(&adc_buf[ADC_HALF_SIZE]);
            Led_Toggle();
        }
    }
}
