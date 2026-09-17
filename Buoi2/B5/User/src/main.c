
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

static uint8_t String_Equal(const char *a, const char *b){
    while (*a && *b){
        if (*a != *b){
            return 0;
        }
        a++;
        b++;
    }
    return (*a == '\0' && *b == '\0');
}

static int String_ToInt(const char *str)
{
    int value = 0;
    while (*str >= '0' && *str <= '9'){
        value = value * 10 + (*str - '0');
        str++;
    }
    return value;
}

static void UART_SendNumber(uint8_t value){
    if (value >= 100){
        UART1_SendChar('1');
        UART1_SendChar('0');
        UART1_SendChar('0');
    }
    else if (value >= 10){
        UART1_SendChar((value / 10) + '0');
        UART1_SendChar((value % 10) + '0');
    }
    else{
        UART1_SendChar(value + '0');
    }
}

static void Process_Command(char *cmd){
    int value;
    if (String_Equal(cmd, "ON")){
        led_state = LED_ON;
        TIM2_PWM_SetDuty(LED_PWM_CHANNEL,pwm_config);
        UART1_SendString("BAT LED, DO SANG = ");
        UART_SendNumber(pwm_config);
        UART1_SendString("%\r\n");
        return;
    }
    if (String_Equal(cmd, "OFF")){
        led_state = LED_OFF;
        TIM2_PWM_SetDuty(LED_PWM_CHANNEL,0);
        UART1_SendString("TAT LED\r\n");
        return;
    }
    value = String_ToInt(cmd);
    if (value < 0 || value > 100){
        UART1_SendString("ERROR: COMMAND\r\n");
        return;
    }
    pwm_config = (uint8_t)value;
    if (led_state == LED_ON){
        TIM2_PWM_SetDuty(LED_PWM_CHANNEL,pwm_config);
    }
    UART1_SendString("Doi Do Sang = ");
    UART_SendNumber(pwm_config);
    UART1_SendString("%\r\n");
}
int main(void){
    char command[UART_CMD_BUFFER_SIZE];
    uint8_t i;
    RCC_Config_72Mhz();
    RCC_Enable_TIM2();
    RCC_Enable_PortA();
    RCC_Enable_UART1();
    GPIO_Config(GPIOA,GPIO_PIN_9, GPIO_MODE_AF_PP);
    GPIO_Config(GPIOA, GPIO_PIN_10, GPIO_MODE_INPUT_FLOATING);
    GPIO_Config(GPIOA,GPIO_PIN_0,GPIO_MODE_AF_PP);
    TIM2_PWM_Init();
    led_state = LED_OFF;
    pwm_config = 50;
    TIM2_PWM_SetDuty(LED_PWM_CHANNEL,0);
    UART1_Init(9600);
    UART1_SendString("\r\nDieu Khien Do Sang Led Bang UART \r\n");
    while (1){
        if (uart_cmd_ready){
            for (i = 0; i < UART_CMD_BUFFER_SIZE; i++){
                command[i] = uart_cmd_buffer[i];
                if (command[i] == '\0'){
                    break;
                }
            }
            uart_cmd_ready = 0;
            Process_Command(command);
        }
    }
}
