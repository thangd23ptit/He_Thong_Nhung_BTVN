
#include <stdint.h>

#define RCC_BASE        0x40021000UL

#define RCC_CR          (*(volatile uint32_t *)(RCC_BASE + 0x00))
#define RCC_CFGR        (*(volatile uint32_t *)(RCC_BASE + 0x04))
#define RCC_APB2ENR     (*(volatile uint32_t *)(RCC_BASE + 0x18))
#define RCC_APB1ENR     (*(volatile uint32_t *)(RCC_BASE + 0x1C))

#define RCC_APB2ENR_IOPAEN    (1U << 2)
#define RCC_APB2ENR_IOPBEN    (1U << 3)
#define RCC_APB2ENR_USART1EN  (1U << 14)

#define RCC_APB1ENR_I2C1EN    (1U << 21)


#define GPIOA_BASE       0x40010800UL

#define GPIOA_CRL        (*(volatile uint32_t *)(GPIOA_BASE + 0x00))
#define GPIOA_CRH        (*(volatile uint32_t *)(GPIOA_BASE + 0x04))
#define GPIOA_IDR        (*(volatile uint32_t *)(GPIOA_BASE + 0x08))
#define GPIOA_ODR        (*(volatile uint32_t *)(GPIOA_BASE + 0x0C))


#define GPIOB_BASE       0x40010C00UL

#define GPIOB_CRL        (*(volatile uint32_t *)(GPIOB_BASE + 0x00))
#define GPIOB_CRH        (*(volatile uint32_t *)(GPIOB_BASE + 0x04))
#define GPIOB_IDR        (*(volatile uint32_t *)(GPIOB_BASE + 0x08))
#define GPIOB_ODR        (*(volatile uint32_t *)(GPIOB_BASE + 0x0C))


#define USART1_BASE      0x40013800UL

#define USART1_SR        (*(volatile uint32_t *)(USART1_BASE + 0x00))
#define USART1_DR        (*(volatile uint32_t *)(USART1_BASE + 0x04))
#define USART1_BRR       (*(volatile uint32_t *)(USART1_BASE + 0x08))
#define USART1_CR1       (*(volatile uint32_t *)(USART1_BASE + 0x0C))
#define USART1_CR2       (*(volatile uint32_t *)(USART1_BASE + 0x10))
#define USART1_CR3       (*(volatile uint32_t *)(USART1_BASE + 0x14))

#define USART_SR_RXNE    (1U << 5)
#define USART_SR_TXE     (1U << 7)

#define USART_CR1_RE     (1U << 2)
#define USART_CR1_TE     (1U << 3)
#define USART_CR1_UE     (1U << 13)


#define I2C1_BASE        0x40005400UL

#define I2C1_CR1         (*(volatile uint32_t *)(I2C1_BASE + 0x00))
#define I2C1_CR2         (*(volatile uint32_t *)(I2C1_BASE + 0x04))
#define I2C1_OAR1        (*(volatile uint32_t *)(I2C1_BASE + 0x08))
#define I2C1_DR          (*(volatile uint32_t *)(I2C1_BASE + 0x10))
#define I2C1_SR1         (*(volatile uint32_t *)(I2C1_BASE + 0x14))
#define I2C1_SR2         (*(volatile uint32_t *)(I2C1_BASE + 0x18))
#define I2C1_CCR         (*(volatile uint32_t *)(I2C1_BASE + 0x1C))
#define I2C1_TRISE       (*(volatile uint32_t *)(I2C1_BASE + 0x20))

#define I2C_CR1_PE       (1U << 0)
#define I2C_CR1_START    (1U << 8)
#define I2C_CR1_STOP     (1U << 9)
#define I2C_CR1_ACK      (1U << 10)
#define I2C_CR1_SWRST    (1U << 15)

#define I2C_SR1_SB       (1U << 0)
#define I2C_SR1_ADDR     (1U << 1)
#define I2C_SR1_BTF      (1U << 2)
#define I2C_SR1_RXNE     (1U << 6)
#define I2C_SR1_TXE      (1U << 7)
#define I2C_SR1_AF       (1U << 10)

#define I2C_SR2_BUSY     (1U << 1)


#define DS1307_ADDR      0x68

#define DS1307_SEC       0x00
#define DS1307_MIN       0x01
#define DS1307_HOUR      0x02
#define DS1307_DAY       0x03
#define DS1307_DATE      0x04
#define DS1307_MONTH     0x05
#define DS1307_YEAR      0x06


#define UART_BUFFER_SIZE 64

volatile char uart_buffer[UART_BUFFER_SIZE];
volatile uint8_t uart_index = 0;

volatile uint8_t rtc_running = 0;


typedef struct
{
    uint8_t second;
    uint8_t minute;
    uint8_t hour;
    uint8_t day;
    uint8_t date;
    uint8_t month;
    uint8_t year;

} RTC_Time;


void delay_ms(uint32_t ms)
{
    volatile uint32_t i;
    volatile uint32_t j;

    for (i = 0; i < ms; i++)
    {
        for (j = 0; j < 8000; j++)
        {
            __asm volatile ("nop");
        }
    }
}


void UART1_Init(void)
{
    RCC_APB2ENR |= RCC_APB2ENR_IOPAEN;
    RCC_APB2ENR |= RCC_APB2ENR_USART1EN;

    GPIOA_CRH &= ~(0xFU << 4);
    GPIOA_CRH |=  (0xBU << 4);

    GPIOA_CRH &= ~(0xFU << 8);
    GPIOA_CRH |=  (0x4U << 8);

    USART1_BRR = 0x0045;

    USART1_CR1 = 0;

    USART1_CR1 |= USART_CR1_TE;
    USART1_CR1 |= USART_CR1_RE;

    USART1_CR1 |= USART_CR1_UE;
}


void UART1_SendChar(char c)
{
    while (!(USART1_SR & USART_SR_TXE))
    {
    }

    USART1_DR = c;
}


void UART1_SendString(const char *str)
{
    while (*str)
    {
        UART1_SendChar(*str++);
    }
}


char UART1_ReadChar(void)
{
    while (!(USART1_SR & USART_SR_RXNE))
    {
    }

    return (char)(USART1_DR & 0xFF);
}


void UART1_Send2(uint8_t value)
{
    UART1_SendChar((value / 10) + '0');
    UART1_SendChar((value % 10) + '0');
}


void I2C1_Init(void)
{
    RCC_APB2ENR |= RCC_APB2ENR_IOPBEN;
    RCC_APB1ENR |= RCC_APB1ENR_I2C1EN;

    GPIOB_CRL &= ~(0xFFU << 24);

    GPIOB_CRL |= (0xFU << 24);
    GPIOB_CRL |= (0xFU << 28);

    I2C1_CR1 = 0;

    I2C1_CR2 = 8;

    I2C1_OAR1 = (1U << 14);

    I2C1_CCR = 40;

    I2C1_TRISE = 9;

    I2C1_CR1 |= I2C_CR1_PE;

    I2C1_CR1 |= I2C_CR1_ACK;
}


uint8_t I2C1_Start(void)
{
    uint32_t timeout = 100000;

    while ((I2C1_SR2 & I2C_SR2_BUSY) && timeout)
    {
        timeout--;
    }

    if (timeout == 0)
    {
        return 0;
    }

    I2C1_CR1 |= I2C_CR1_START;

    timeout = 100000;

    while (!(I2C1_SR1 & I2C_SR1_SB) && timeout)
    {
        timeout--;
    }

    if (timeout == 0)
    {
        return 0;
    }

    return 1;
}


void I2C1_Stop(void)
{
    I2C1_CR1 |= I2C_CR1_STOP;
}


uint8_t I2C1_SendAddress(uint8_t address)
{
    uint32_t timeout = 100000;

    I2C1_DR = address;

    while (!(I2C1_SR1 & I2C_SR1_ADDR) && timeout)
    {
        timeout--;
    }

    if (timeout == 0)
    {
        I2C1_CR1 |= I2C_CR1_STOP;

        return 0;
    }

    (void)I2C1_SR1;
    (void)I2C1_SR2;

    return 1;
}


uint8_t I2C1_WriteByte(uint8_t data)
{
    uint32_t timeout = 100000;

    I2C1_DR = data;

    while (!(I2C1_SR1 & I2C_SR1_TXE) && timeout)
    {
        timeout--;
    }

    if (timeout == 0)
    {
        return 0;
    }

    while (!(I2C1_SR1 & I2C_SR1_BTF) && timeout)
    {
        timeout--;
    }

    if (timeout == 0)
    {
        return 0;
    }

    return 1;
}


uint8_t I2C1_ReadByte(uint8_t ack)
{
    uint32_t timeout = 100000;
    uint8_t data;

    if (ack)
    {
        I2C1_CR1 |= I2C_CR1_ACK;
    }
    else
    {
        I2C1_CR1 &= ~I2C_CR1_ACK;
    }

    while (!(I2C1_SR1 & I2C_SR1_RXNE) && timeout)
    {
        timeout--;
    }

    if (timeout == 0)
    {
        return 0;
    }

    data = (uint8_t)I2C1_DR;

    return data;
}


uint8_t DS1307_WriteReg(uint8_t reg, uint8_t data)
{
    if (!I2C1_Start())
    {
        return 0;
    }

    if (!I2C1_SendAddress((DS1307_ADDR << 1) | 0))
    {
        I2C1_Stop();
        return 0;
    }

    if (!I2C1_WriteByte(reg))
    {
        I2C1_Stop();
        return 0;
    }

    if (!I2C1_WriteByte(data))
    {
        I2C1_Stop();
        return 0;
    }

    I2C1_Stop();

    return 1;
}


uint8_t DS1307_ReadReg(uint8_t reg)
{
    uint8_t data;

    if (!I2C1_Start())
    {
        return 0;
    }

    if (!I2C1_SendAddress((DS1307_ADDR << 1) | 0))
    {
        I2C1_Stop();
        return 0;
    }

    if (!I2C1_WriteByte(reg))
    {
        I2C1_Stop();
        return 0;
    }

    I2C1_CR1 |= I2C_CR1_START;

    while (!(I2C1_SR1 & I2C_SR1_SB))
    {
    }

    if (!I2C1_SendAddress((DS1307_ADDR << 1) | 1))
    {
        I2C1_Stop();
        return 0;
    }

    I2C1_CR1 &= ~I2C_CR1_ACK;

    data = I2C1_ReadByte(0);

    I2C1_Stop();

    I2C1_CR1 |= I2C_CR1_ACK;

    return data;
}


uint8_t BCD_To_DEC(uint8_t bcd)
{
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}


uint8_t DEC_To_BCD(uint8_t dec)
{
    return ((dec / 10) << 4) | (dec % 10);
}


uint8_t DS1307_ReadTime(RTC_Time *time)
{
    if (!I2C1_Start())
    {
        return 0;
    }

    if (!I2C1_SendAddress((DS1307_ADDR << 1) | 0))
    {
        I2C1_Stop();
        return 0;
    }

    if (!I2C1_WriteByte(DS1307_SEC))
    {
        I2C1_Stop();
        return 0;
    }

    I2C1_CR1 |= I2C_CR1_START;

    while (!(I2C1_SR1 & I2C_SR1_SB))
    {
    }

    if (!I2C1_SendAddress((DS1307_ADDR << 1) | 1))
    {
        I2C1_Stop();
        return 0;
    }

    I2C1_CR1 |= I2C_CR1_ACK;

    time->second = I2C1_ReadByte(1);
    time->minute = I2C1_ReadByte(1);
    time->hour   = I2C1_ReadByte(1);
    time->day    = I2C1_ReadByte(1);
    time->date   = I2C1_ReadByte(1);
    time->month  = I2C1_ReadByte(1);

    I2C1_CR1 &= ~I2C_CR1_ACK;

    time->year = I2C1_ReadByte(0);

    I2C1_Stop();

    I2C1_CR1 |= I2C_CR1_ACK;

    time->second = BCD_To_DEC(time->second & 0x7F);
    time->minute = BCD_To_DEC(time->minute & 0x7F);
    time->hour   = BCD_To_DEC(time->hour & 0x3F);
    time->day    = BCD_To_DEC(time->day & 0x07);
    time->date   = BCD_To_DEC(time->date & 0x3F);
    time->month  = BCD_To_DEC(time->month & 0x1F);
    time->year   = BCD_To_DEC(time->year);

    return 1;
}


uint8_t DS1307_SetTime(
    uint8_t second,
    uint8_t minute,
    uint8_t hour,
    uint8_t day,
    uint8_t date,
    uint8_t month,
    uint8_t year
)
{
    if (!I2C1_Start())
    {
        return 0;
    }

    if (!I2C1_SendAddress((DS1307_ADDR << 1) | 0))
    {
        I2C1_Stop();
        return 0;
    }

    if (!I2C1_WriteByte(DS1307_SEC))
    {
        I2C1_Stop();
        return 0;
    }

    if (!I2C1_WriteByte(DEC_To_BCD(second) & 0x7F))
    {
        I2C1_Stop();
        return 0;
    }

    if (!I2C1_WriteByte(DEC_To_BCD(minute)))
    {
        I2C1_Stop();
        return 0;
    }

    if (!I2C1_WriteByte(DEC_To_BCD(hour) & 0x3F))
    {
        I2C1_Stop();
        return 0;
    }

    if (!I2C1_WriteByte(DEC_To_BCD(day)))
    {
        I2C1_Stop();
        return 0;
    }

    if (!I2C1_WriteByte(DEC_To_BCD(date)))
    {
        I2C1_Stop();
        return 0;
    }

    if (!I2C1_WriteByte(DEC_To_BCD(month)))
    {
        I2C1_Stop();
        return 0;
    }

    if (!I2C1_WriteByte(DEC_To_BCD(year)))
    {
        I2C1_Stop();
        return 0;
    }

    I2C1_Stop();

    return 1;
}


void RTC_PrintTime(void)
{
    RTC_Time time;

    if (!DS1307_ReadTime(&time))
    {
        UART1_SendString("\r\nRTC READ ERROR\r\n");
        return;
    }

    UART1_Send2(time.date);
    UART1_SendChar('/');

    UART1_Send2(time.month);
    UART1_SendString("/20");

    UART1_Send2(time.year);

    UART1_SendChar(' ');

    UART1_Send2(time.hour);
    UART1_SendChar(':');

    UART1_Send2(time.minute);
    UART1_SendChar(':');

    UART1_Send2(time.second);

    UART1_SendString("\r\n");
}


uint8_t StringEqual(
    const char *a,
    const char *b
)
{
    uint16_t i = 0;

    while (a[i] != '\0' && b[i] != '\0')
    {
        if (a[i] != b[i])
        {
            return 0;
        }

        i++;
    }

    if (a[i] == '\0' && b[i] == '\0')
    {
        return 1;
    }

    return 0;
}


uint8_t ToNumber(
    const char *str,
    uint8_t start,
    uint8_t count
)
{
    uint8_t value = 0;
    uint8_t i;

    for (i = 0; i < count; i++)
    {
        if (str[start + i] < '0' ||
            str[start + i] > '9')
        {
            return 255;
        }

        value = value * 10 +
                (str[start + i] - '0');
    }

    return value;
}


void ProcessSetCommand(char *cmd)
{
    uint8_t date;
    uint8_t month;
    uint8_t year;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;

    date   = ToNumber(cmd, 4, 2);
    month  = ToNumber(cmd, 7, 2);
    year   = ToNumber(cmd, 10, 2);

    hour   = ToNumber(cmd, 13, 2);
    minute = ToNumber(cmd, 16, 2);
    second = ToNumber(cmd, 19, 2);

    if (date == 255 ||
        month == 255 ||
        year == 255 ||
        hour == 255 ||
        minute == 255 ||
        second == 255)
    {
        UART1_SendString("\r\nSET FORMAT ERROR\r\n");
        return;
    }

    if (date < 1 || date > 31)
    {
        UART1_SendString("\r\nINVALID DATE\r\n");
        return;
    }

    if (month < 1 || month > 12)
    {
        UART1_SendString("\r\nINVALID MONTH\r\n");
        return;
    }

    if (hour > 23)
    {
        UART1_SendString("\r\nINVALID HOUR\r\n");
        return;
    }

    if (minute > 59)
    {
        UART1_SendString("\r\nINVALID MINUTE\r\n");
        return;
    }

    if (second > 59)
    {
        UART1_SendString("\r\nINVALID SECOND\r\n");
        return;
    }

    if (DS1307_SetTime(
            second,
            minute,
            hour,
            1,
            date,
            month,
            year))
    {
        UART1_SendString("\r\nRTC SET OK\r\n");

        rtc_running = 1;
    }
    else
    {
        UART1_SendString("\r\nRTC SET ERROR\r\n");

        rtc_running = 0;
    }
}


void ProcessCommand(char *cmd)
{
    if (cmd[0] == 'S' &&
        cmd[1] == 'E' &&
        cmd[2] == 'T' &&
        cmd[3] == ' ')
    {
        ProcessSetCommand(cmd);
    }
    else if (StringEqual(cmd, "TIME"))
    {
        RTC_PrintTime();
    }
    else
    {
        UART1_SendString("\r\nUNKNOWN COMMAND\r\n");
    }
}


int main(void)
{
    RTC_Time time;

    uint8_t last_second = 255;

    UART1_Init();

    I2C1_Init();

    UART1_SendString("\r\n");
    UART1_SendString("============================\r\n");
    UART1_SendString(" STM32F103 + DS1307 RTC\r\n");
    UART1_SendString("============================\r\n");

    UART1_SendString("I2C1: PB6=SCL PB7=SDA\r\n");
    UART1_SendString("UART1: 115200\r\n");

    UART1_SendString("\r\nCommand:\r\n");
    UART1_SendString("SET DD/MM/YY HH:MM:SS!\r\n");
    UART1_SendString("TIME!\r\n");

    UART1_SendString("\r\n> ");

    while (1)
    {
        if (USART1_SR & USART_SR_RXNE)
        {
            char c;

            c = (char)(USART1_DR & 0xFF);

            if (c == '!')
            {
                uart_buffer[uart_index] = '\0';

                UART1_SendString("\r\n");

                ProcessCommand((char *)uart_buffer);

                uart_index = 0;

                last_second = 255;

                UART1_SendString("> ");
            }
            else if (c != '\r' && c != '\n')
            {
                if (uart_index < UART_BUFFER_SIZE - 1)
                {
                    uart_buffer[uart_index++] = c;

                    UART1_SendChar(c);
                }
            }
        }

        if (rtc_running)
        {
            if (DS1307_ReadTime(&time))
            {
                if (time.second != last_second)
                {
                    last_second = time.second;

                    UART1_Send2(time.date);
                    UART1_SendChar('/');

                    UART1_Send2(time.month);

                    UART1_SendString("/20");

                    UART1_Send2(time.year);

                    UART1_SendChar(' ');

                    UART1_Send2(time.hour);
                    UART1_SendChar(':');

                    UART1_Send2(time.minute);
                    UART1_SendChar(':');

                    UART1_Send2(time.second);

                    UART1_SendString("\r\n");
                }
            }
        }
    }
}

