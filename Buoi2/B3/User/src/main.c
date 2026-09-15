#include <stdint.h>

#include "RCC.h"
#include "UART.h"
#include "ADC.h"
#include "TIM.h"

#define VREF_MV    3300UL
#define ADC_MAX    4095UL

void UART1_SendNumber(uint32_t number);
void UART1_SendVoltage(uint32_t voltage_mV);

void UART1_SendNumber(uint32_t number)
{
    char buffer[10];
    uint8_t i = 0;

    if (number == 0)
    {
        UART1_SendChar('0');
        return;
    }

    while (number > 0)
    {
        buffer[i++] = (number % 10) + '0';
        number /= 10;
    }

    while (i > 0)
    {
        UART1_SendChar(buffer[--i]);
    }
}

void UART1_SendVoltage(uint32_t voltage_mV)
{
    uint32_t volt;
    uint32_t decimal;

    volt = voltage_mV / 1000;
    decimal = voltage_mV % 1000;

    UART1_SendNumber(volt);
    UART1_SendChar('.');

    UART1_SendChar((decimal / 100) + '0');
    UART1_SendChar(((decimal / 10) % 10) + '0');
    UART1_SendChar((decimal % 10) + '0');

    UART1_SendString(" V");
}

int main(void)
{   
    RCC_Enable_PortA();
    RCC_Enable_ADC01();
    uint16_t adc_value;
    uint32_t voltage_mV;

    RCC_Config_72Mhz();

    UART1_Init(9600);

    UART1_SendString("\r\nDoc gia tri dien ap qua UART\r\n");

    ADC01_CH3_Init();

    UART1_SendString("Da Khoi Tao Cho ADC1 Kenh 3\r\n");

    while (1)
    {
        adc_value = ADC01_CH3_Read();

        voltage_mV = ((uint32_t)adc_value * VREF_MV) / ADC_MAX;

        UART1_SendString("ADC = ");
        UART1_SendNumber(adc_value);

        UART1_SendString(" | Voltage = ");
        UART1_SendVoltage(voltage_mV);

        UART1_SendString("\r\n");

        delay_ms(9000);
    }
}