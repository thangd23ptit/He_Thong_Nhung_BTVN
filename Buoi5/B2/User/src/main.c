
#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdint.h>
#include <stdio.h>

/*====================================================
 * 1. KHAI BAO CAU HINH
 *====================================================*/

#define BUTTON_PORT       GPIOB
#define BUTTON_PIN        GPIO_Pin_12
#define BUTTON_EXTI_LINE  EXTI_Line12
#define BUTTON_EXTI_IRQ   EXTI15_10_IRQn

#define UART_BAUDRATE     115200
#define QUEUE_LENGTH      10

#define DEBOUNCE_MS  100U

static volatile TickType_t lastButtonTick = 0;

/* Queue truyen gia tri dem so lan nhan nut */
static QueueHandle_t CountQueue = NULL;

/* Bien dem so lan nhan nut */
static volatile uint32_t pressCount = 0;

/*====================================================
 * 2. KHOI TAO UART1
 * PA9  = USART1_TX
 * PA10 = USART1_RX
 *====================================================*/

static void USART1_Init(void)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;

    /* Cap clock GPIOA va USART1 */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA |
        RCC_APB2Periph_USART1,
        ENABLE
    );

    /* PA9: TX, Alternate Function Push-Pull */
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    /* PA10: RX, Input Floating */
    gpio.GPIO_Pin = GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    /* Cau hinh UART 115200, 8N1 */
    USART_StructInit(&usart);
    usart.USART_BaudRate = UART_BAUDRATE;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    usart.USART_HardwareFlowControl =
        USART_HardwareFlowControl_None;

    USART_Init(USART1, &usart);

    /* Bat USART1 */
    USART_Cmd(USART1, ENABLE);
}

/*====================================================
 * 3. HAM TRUYEN CHUOI QUA UART1
 *====================================================*/

static void USART1_SendString(const char *str)
{
    while (*str != '\0')
    {
        /* Cho thanh ghi truyen trong */
        while (USART_GetFlagStatus(
                   USART1, USART_FLAG_TXE) == RESET)
        {
        }

        /* Gui mot ky tu */
        USART_SendData(USART1, (uint16_t)*str);

        str++;
    }

    /* Cho bit cuoi cung truyen xong */
    while (USART_GetFlagStatus(
               USART1, USART_FLAG_TC) == RESET)
    {
    }
}

/*====================================================
 * 4. KHOI TAO GPIO NUT NHAN
 *====================================================*/

static void GPIO_Button_Init(void)
{
    GPIO_InitTypeDef gpio;

    /* Cap clock GPIOB va AFIO */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOB |
        RCC_APB2Periph_AFIO,
        ENABLE
    );

    /* PB12 la input pull-up */
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = BUTTON_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;

    GPIO_Init(BUTTON_PORT, &gpio);
}

/*====================================================
 * 5. KHOI TAO EXTI12
 *====================================================*/

static void EXTI_Button_Init(void)
{
    EXTI_InitTypeDef exti;

    /* Anh xa EXTI12 voi chan PB12 */
    GPIO_EXTILineConfig(
        GPIO_PortSourceGPIOB,
        GPIO_PinSource12
    );

    /* Xoa co ngat cu */
    EXTI_ClearITPendingBit(BUTTON_EXTI_LINE);

    /* Cau hinh ngat canh xuong */
    EXTI_StructInit(&exti);
    exti.EXTI_Line = BUTTON_EXTI_LINE;
    exti.EXTI_Mode = EXTI_Mode_Interrupt;
    exti.EXTI_Trigger = EXTI_Trigger_Falling;

    /* Tam thoi chua cho phep EXTI12 */
    exti.EXTI_LineCmd = DISABLE;

    EXTI_Init(&exti);
}

/*====================================================
 * 6. NGAT NGOAI EXTI15_10
 *====================================================*/

void EXTI15_10_IRQHandler(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    uint32_t count;
    TickType_t currentTick;

    if (EXTI_GetITStatus(BUTTON_EXTI_LINE) != RESET)
    {
        /* Lay thoi gian hien tai trong ISR */
        currentTick = xTaskGetTickCountFromISR();

        /* Loc rung: chi chap nhan neu da qua 50 ms */
        if ((TickType_t)(currentTick - lastButtonTick) >=
            pdMS_TO_TICKS(DEBOUNCE_MS))
        {
            lastButtonTick = currentTick;

            /* Tang so lan nhan nut */
            pressCount++;
            count = pressCount;

            /* Gui gia tri dem vao Queue */
            xQueueSendFromISR(
                CountQueue,
                &count,
                &xHigherPriorityTaskWoken
            );
        }

        /* Xoa co ngat */
        EXTI_ClearITPendingBit(BUTTON_EXTI_LINE);

        /* Chuyen ngu canh neu can */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/*====================================================
 * 7. TASK UART
 * Nhan du lieu tu Queue va truyen len may tinh
 *====================================================*/

static void TaskUART(void *argument)
{
    uint32_t receivedCount;
    char message[64];
    NVIC_InitTypeDef nvic;

    (void)argument;

    /* Cau hinh NVIC cho ngat EXTI */
    nvic.NVIC_IRQChannel = BUTTON_EXTI_IRQ;
    nvic.NVIC_IRQChannelPreemptionPriority = 5;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&nvic);

    /* Cho phep EXTI12 sau khi scheduler da chay */
    EXTI_ClearITPendingBit(BUTTON_EXTI_LINE);
    EXTI_InitTypeDef exti;
    EXTI_StructInit(&exti);
    exti.EXTI_Line = BUTTON_EXTI_LINE;
    exti.EXTI_Mode = EXTI_Mode_Interrupt;
    exti.EXTI_Trigger = EXTI_Trigger_Falling;
    exti.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti);

    USART1_SendString("System ready.\r\n");
    USART1_SendString("Press button on PB12.\r\n");

    for (;;)
    {
        /* Cho du lieu tu Queue.
         * Neu Queue rong, TaskUART vao Blocked.
         * Khi ISR gui du lieu, task duoc danh thuc.
         */
        if (xQueueReceive(
                CountQueue,
                &receivedCount,
                portMAX_DELAY) == pdPASS)
        {
            /* Dinh dang gia tri dem thanh chuoi */
            snprintf(
                message,
                sizeof(message),
                "So lan nhan nut: %lu\r\n",
                (unsigned long)receivedCount
            );

            /* Truyen chuoi qua USART1 */
            USART1_SendString(message);
        }
    }
}

/*====================================================
 * 8. HAM MAIN
 *====================================================*/

int main(void)
{
    /* Khoi tao clock va he thong co ban */
    SystemInit();

    /* Chon 4 bit uu tien preemption cho Cortex-M3 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* Khoi tao UART1 */
    USART1_Init();

    /* Khoi tao GPIO nut nhan */
    GPIO_Button_Init();

    /* Cau hinh EXTI12, chua cho phep ngat */
    EXTI_Button_Init();

    /* Tao Queue chua 10 gia tri uint32_t */
    CountQueue = xQueueCreate(
        QUEUE_LENGTH,
        sizeof(uint32_t)
    );

    /* Kiem tra tao Queue */
    if (CountQueue == NULL)
    {
        while (1)
        {
        }
    }

    /* Tao TaskUART
     * Stack depth = 256 don vi stack
     * Priority = 3
     */
    if (xTaskCreate(
            TaskUART,
            "UART",
            256,
            NULL,
            3,
            NULL) != pdPASS)
    {
        while (1)
        {
        }
    }

    /* Bat dau bo lap lich FreeRTOS */
    vTaskStartScheduler();

    /* Neu scheduler khong khoi dong duoc */
    while (1)
    {
    }
}
