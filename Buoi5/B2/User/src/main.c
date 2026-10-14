
#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdint.h>
#include <stdio.h>

/*====================================================
 * 1. KHAI BAO CAU HINH
 *====================================================*/

/* Nut nhan duoc noi voi PB12 */
#define BUTTON_PORT GPIOB
#define BUTTON_PIN GPIO_Pin_12

/* EXTI12 duoc su dung de tao ngat ngoai */
#define BUTTON_EXTI_LINE EXTI_Line12
#define BUTTON_EXTI_IRQ EXTI15_10_IRQn

/* Toc do truyen UART1: 115200 baud */
#define UART_BAUDRATE 115200

/* So phan tu toi da cua Queue */
#define QUEUE_LENGTH 10

/* Thoi gian chong doi nut nhan: 100 ms */
#define DEBOUNCE_MS 100U

/* Luu thoi diem xu ly nut nhan gan nhat */
static volatile TickType_t lastButtonTick = 0;

/* Queue truyen so lan nhan nut tu ISR sang TaskUART */
static QueueHandle_t CountQueue = NULL;

/* Bien dem tong so lan nhan nut */
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

    /* Cap clock cho GPIOA va USART1 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    /* Cau hinh PA9 lam chan truyen UART */
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    /* Cau hinh PA10 lam chan nhan UART */
    gpio.GPIO_Pin = GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    /* Khoi tao cau hinh UART mac dinh */
    USART_StructInit(&usart);

    /* Cau hinh toc do truyen 115200 baud */
    usart.USART_BaudRate = UART_BAUDRATE;

    /* Moi khung du lieu co 8 bit */
    usart.USART_WordLength = USART_WordLength_8b;

    /* Su dung 1 bit Stop */
    usart.USART_StopBits = USART_StopBits_1;

    /* Khong su dung bit Parity */
    usart.USART_Parity = USART_Parity_No;

    /* Cho phep ca truyen va nhan du lieu */
    usart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    /* Khong su dung dieu khien luong bang phan cung */
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    /* Ap dung cau hinh cho USART1 */
    USART_Init(USART1, &usart);

    /* Bat ngoai vi USART1 */
    USART_Cmd(USART1, ENABLE);
}

/*====================================================
 * 3. HAM TRUYEN CHUOI QUA UART1
 *====================================================*/

static void USART1_SendString(const char *str)
{
    /* Duyet tung ky tu cho den khi gap ky tu ket thuc chuoi */
    while (*str != '\0')
    {
        /* Cho thanh ghi du lieu truyen rong */
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
        {
        }

        /* Dua ky tu vao thanh ghi truyen USART1 */
        USART_SendData(USART1, (uint16_t)*str);

        /* Chuyen sang ky tu tiep theo */
        str++;
    }

    /* Cho bit cuoi cung truyen xong hoan toan */
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
    {
    }
}

/*====================================================
 * 4. KHOI TAO GPIO NUT NHAN
 *====================================================*/

static void GPIO_Button_Init(void)
{
    GPIO_InitTypeDef gpio;

    /* Cap clock cho GPIOB va bo AFIO */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    /* Cau hinh PB12 la ngo vao keo len noi */
    GPIO_StructInit(&gpio);
    gpio.GPIO_Pin = BUTTON_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;

    /* Ap dung cau hinh GPIO */
    GPIO_Init(BUTTON_PORT, &gpio);
}

/*====================================================
 * 5. KHOI TAO EXTI12
 *====================================================*/

static void EXTI_Button_Init(void)
{
    EXTI_InitTypeDef exti;

    /* Anh xa EXTI12 den chan PB12 */
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource12);

    /* Xoa co ngat ton dong truoc khi khoi tao */
    EXTI_ClearITPendingBit(BUTTON_EXTI_LINE);

    /* Khoi tao cau hinh EXTI mac dinh */
    EXTI_StructInit(&exti);

    /* Lua chon duong ngat EXTI12 */
    exti.EXTI_Line = BUTTON_EXTI_LINE;

    /* Su dung che do ngat */
    exti.EXTI_Mode = EXTI_Mode_Interrupt;

    /* Phat sinh ngat khi tin hieu chuyen tu cao xuong thap */
    exti.EXTI_Trigger = EXTI_Trigger_Falling;

    /* Chua cho phep ngat, TaskUART se bat ngat sau */
    exti.EXTI_LineCmd = DISABLE;

    /* Ap dung cau hinh EXTI */
    EXTI_Init(&exti);
}

/*====================================================
 * 6. HAM PHUC VU NGAT EXTI15_10
 *====================================================*/

void EXTI15_10_IRQHandler(void)
{
    /* Co yeu cau chuyen ngu canh sau khi thoat ISR */
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    /* Bien tam luu so lan nhan nut */
    uint32_t count;

    /* Thoi diem hien tai tinh theo tick FreeRTOS */
    TickType_t currentTick;

    /* Kiem tra ngat co phai den tu EXTI12 hay khong */
    if (EXTI_GetITStatus(BUTTON_EXTI_LINE) != RESET)
    {
        /* Lay tick hien tai an toan trong ISR */
        currentTick = xTaskGetTickCountFromISR();

        /* Chi chap nhan su kien neu da qua thoi gian chong doi */
        if ((TickType_t)(currentTick - lastButtonTick) >= pdMS_TO_TICKS(DEBOUNCE_MS))
        {
            /* Cap nhat thoi diem nhan nut gan nhat */
            lastButtonTick = currentTick;

            /* Tang bien dem so lan nhan nut */
            pressCount++;

            /* Sao chep gia tri dem vao bien cuc bo */
            count = pressCount;

            /* Gui so lan nhan nut vao Queue tu ISR */
            xQueueSendFromISR(CountQueue, &count, &xHigherPriorityTaskWoken);
        }

        /* Xoa co ngat EXTI12 */
        EXTI_ClearITPendingBit(BUTTON_EXTI_LINE);

        /* Yeu cau chuyen ngu canh neu Task uu tien cao hon duoc danh thuc */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

/*====================================================
 * 7. TASK UART
 * Nhan du lieu tu Queue va truyen len may tinh
 *====================================================*/

static void TaskUART(void *argument)
{
    /* Bien nhan so lan nhan nut tu Queue */
    uint32_t receivedCount;

    /* Bo dem dung de dinh dang chuoi UART */
    char message[64];

    /* Bien cau hinh NVIC */
    NVIC_InitTypeDef nvic;

    (void)argument;

    /* Cau hinh NVIC cho ngat EXTI15_10 */
    nvic.NVIC_IRQChannel = BUTTON_EXTI_IRQ;
    nvic.NVIC_IRQChannelPreemptionPriority = 5;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    /* Bat ngat EXTI12 sau khi Scheduler da chay */
    EXTI_ClearITPendingBit(BUTTON_EXTI_LINE);

    /* Khoi tao cau hinh EXTI truoc khi cho phep ngat */
    EXTI_InitTypeDef exti;
    EXTI_StructInit(&exti);
    exti.EXTI_Line = BUTTON_EXTI_LINE;
    exti.EXTI_Mode = EXTI_Mode_Interrupt;
    exti.EXTI_Trigger = EXTI_Trigger_Falling;
    exti.EXTI_LineCmd = ENABLE;
    EXTI_Init(&exti);

    /* Gui thong bao khoi dong den may tinh */
    USART1_SendString("==Nhom 4==\r\n");
    USART1_SendString("Nhan nut tai PB12\r\n");

    for (;;)
    {
        /* Cho du lieu moi tu Queue.
         * Khi Queue rong, TaskUART chuyen sang trang thai Blocked.
         * Khi ISR gui du lieu, Task co the duoc danh thuc.
         */
        if (xQueueReceive(CountQueue, &receivedCount, portMAX_DELAY) == pdPASS)
        {
            /* Chuyen so lan nhan nut thanh chuoi ky tu */
            snprintf(message, sizeof(message), "So lan nhan nut: %lu\r\n", (unsigned long)receivedCount);

            /* Truyen ket qua qua UART1 */
            USART1_SendString(message);
        }
    }
}

/*====================================================
 * 8. HAM MAIN
 *====================================================*/

int main(void)
{
    /* Khoi tao he thong clock va cac thanh phan co ban */
    SystemInit();

    /* Cau hinh nhom uu tien NVIC cho Cortex-M3 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    /* Khoi tao UART1 de giao tiep voi may tinh */
    USART1_Init();

    /* Khoi tao GPIO cho nut nhan PB12 */
    GPIO_Button_Init();

    /* Khoi tao EXTI12 nhung chua cho phep ngat */
    EXTI_Button_Init();

    /* Tao Queue co 10 phan tu, moi phan tu la uint32_t */
    CountQueue = xQueueCreate(QUEUE_LENGTH, sizeof(uint32_t));

    /* Kiem tra Queue da duoc tao thanh cong hay chua */
    if (CountQueue == NULL)
    {
        while (1)
        {
            /* Dung neu khong du bo nho tao Queue */
        }
    }

    /* Tao TaskUART:
     * Stack depth = 256 word
     * Muc uu tien = 3
     */
    if (xTaskCreate(TaskUART, "UART", 256, NULL, 3, NULL) != pdPASS)
    {
        while (1)
        {
            /* Dung neu tao Task that bai */
        }
    }

    /* Khoi dong bo lap lich FreeRTOS */
    vTaskStartScheduler();

    /* Neu Scheduler khong chay duoc thi dung tai day */
    while (1)
    {
    }
}
