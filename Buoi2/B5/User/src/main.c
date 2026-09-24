#include <stdint.h>

#include "RCC.h"
#include "GPIO.h"
#include "UART.h"
#include "TIM.h"

#define LED_PWM_CHANNEL TIM2_CH1

#define LED_OFF 0
#define LED_ON  1

static uint8_t led_state = LED_OFF;
static uint8_t pwm_config = 50;

static uint8_t String_Equal(const char *a, const char *b)
{
    while (*a && *b)
    {
        if (*a != *b)
        {
            return 0;
        }

        a++;
        b++;
    }

    return (*a == '\0' && *b == '\0');
}

static void Remove_Line_Ending(char *str)
{
    uint8_t i = 0;

    while (str[i] != '\0')
    {
        if (str[i] == '\r' || str[i] == '\n')
        {
            str[i] = '\0';
            break;
        }

        i++;
    }
}

static uint8_t String_IsNumber(const char *str)
{
    uint8_t has_digit = 0;

    while (*str != '\0')
    {
        if (*str < '0' || *str > '9')
        {
            return 0;
        }

        has_digit = 1;
        str++;
    }

    return has_digit;
}

static int String_ToInt(const char *str)
{
    int value = 0;

    while (*str >= '0' && *str <= '9')
    {
        value = value * 10 + (*str - '0');
        str++;
    }

    return value;
}

static void UART_SendNumber(uint8_t value)
{
    if (value >= 100)
    {
        UART1_SendChar('1');
        UART1_SendChar('0');
        UART1_SendChar('0');
    }
    else if (value >= 10)
    {
        UART1_SendChar((value / 10) + '0');
        UART1_SendChar((value % 10) + '0');
    }
    else
    {
        UART1_SendChar(value + '0');
    }
}

static void Process_Command(char *cmd)
{
    int value;

    Remove_Line_Ending(cmd);

    if (String_Equal(cmd, "ON") || String_Equal(cmd, "on"))
    {
        led_state = LED_ON;

        TIM2_PWM_SetDuty(LED_PWM_CHANNEL, pwm_config);

        UART1_SendString("LED ON, PWM=");
        UART_SendNumber(pwm_config);
        UART1_SendString("%\r\n");

        return;
    }

    if (String_Equal(cmd, "OFF") || String_Equal(cmd, "off"))
    {
        led_state = LED_OFF;

        TIM2_PWM_SetDuty(LED_PWM_CHANNEL, 0);

        UART1_SendString("LED OFF\r\n");

        return;
    }

    if (String_Equal(cmd, "Status") ||
        String_Equal(cmd, "STATUS") ||
        String_Equal(cmd, "status"))
    {
        UART1_SendString("STATUS: ");

        if (led_state == LED_ON)
        {
            UART1_SendString("ON, PWM=");
            UART_SendNumber(pwm_config);
            UART1_SendString("%\r\n");
        }
        else
        {
            UART1_SendString("OFF, PWM_CONFIG=");
            UART_SendNumber(pwm_config);
            UART1_SendString("%\r\n");
        }

        return;
    }

    if (!String_IsNumber(cmd))
    {
        UART1_SendString("ERROR: COMMAND\r\n");
        return;
    }

    value = String_ToInt(cmd);

    if (value < 0 || value > 100)
    {
        UART1_SendString("ERROR: PWM 0-100\r\n");
        return;
    }

    pwm_config = (uint8_t)value;

    if (led_state == LED_ON)
    {
        TIM2_PWM_SetDuty(LED_PWM_CHANNEL, pwm_config);
    }

    UART1_SendString("PWM CONFIG=");
    UART_SendNumber(pwm_config);
    UART1_SendString("%\r\n");
}

int main(void)
{
    char command[UART_CMD_BUFFER_SIZE];
    uint8_t i;

    RCC_Config_72Mhz();

    RCC_Enable_TIM2();
    RCC_Enable_PortA();

    GPIO_Config(
        GPIOA,
        GPIO_PIN_0,
        GPIO_MODE_AF_PP
    );

    TIM2_PWM_Init();

    led_state = LED_OFF;
    pwm_config = 50;

    TIM2_PWM_SetDuty(
        LED_PWM_CHANNEL,
        0
    );

    UART1_Init(9600);

    UART1_SendString("\r\n");
    UART1_SendString("================================\r\n");
    UART1_SendString("UART LED PWM CONTROL\r\n");
    UART1_SendString("Commands:\r\n");
    UART1_SendString("ON\r\n");
    UART1_SendString("OFF\r\n");
    UART1_SendString("Status\r\n");
    UART1_SendString("0-100\r\n");
    UART1_SendString("================================\r\n");

    while (1)
    {
        if (uart_cmd_ready)
        {
            for (i = 0; i < UART_CMD_BUFFER_SIZE - 1; i++)
            {
                command[i] = uart_cmd_buffer[i];

                if (uart_cmd_buffer[i] == '\0')
                {
                    break;
                }
            }

            command[UART_CMD_BUFFER_SIZE - 1] = '\0';

            uart_cmd_ready = 0;

            Process_Command(command);
        }
    }
}
