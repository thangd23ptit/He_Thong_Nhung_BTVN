
#include "stm32f10x.h" // Thu vien ngoai vi STM32F103
#include "FreeRTOS.h"  // Thu vien FreeRTOS
#include "task.h"      // Thu vien quan ly Task
#include "queue.h"     // Thu vien Queue
#include <stdint.h>    // Thu vien kieu du lieu so nguyen
#include <stdio.h>     // Thu vien ham snprintf

#define BUTTON_PORT GPIOB              // Cong GPIO ket noi nut nhan
#define BUTTON_PIN GPIO_Pin_12         // Chan PB12 ket noi nut nhan
#define BUTTON_EXTI_LINE EXTI_Line12   // Duong ngat ngoai EXTI12
#define BUTTON_EXTI_IRQ EXTI15_10_IRQn // Kenh NVIC quan ly EXTI10 den EXTI15

#define UART_BAUDRATE 115200                   // Toc do truyen UART1
#define QUEUE_LENGTH 10                        // So phan tu toi da cua Queue
#define DEBOUNCE_MS 100U                       // Thoi gian chong doi nut nhan 100 ms
static volatile TickType_t lastButtonTick = 0; // Luu thoi diem xu ly nut nhan gan nhat
static volatile uint8_t firstPress = 1;        // Danh dau lan nhan hop le dau tien
static volatile uint32_t pressCount = 0;       // Bien dem so lan nhan nut
static QueueHandle_t CountQueue = NULL;        // Queue truyen du lieu tu ISR sang TaskUART

static void USART1_Init(void);                  // Khai bao ham khoi tao USART1
static void USART1_SendString(const char *str); // Khai bao ham gui chuoi qua USART1
static void GPIO_Button_Init(void);             // Khai bao ham khoi tao GPIO nut nhan
static void EXTI_Button_Init(void);             // Khai bao ham khoi tao EXTI12
static void NVIC_Button_Init(void);             // Khai bao ham cau hinh NVIC
static void TaskUART(void *argument);           // Khai bao Task nhan Queue va gui UART
void EXTI15_10_IRQHandler(void);                // Khai bao ham phuc vu ngat EXTI15_10

static void USART1_Init(void)                                                     // Ham khoi tao USART1
{                                                                                 // Bat dau ham USART1_Init
    GPIO_InitTypeDef gpio;                                                        // Bien cau hinh GPIO
    USART_InitTypeDef usart;                                                      // Bien cau hinh USART
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE); // Cap clock cho GPIOA va USART1
    GPIO_StructInit(&gpio);                                                       // Khoi tao cau hinh GPIO mac dinh
    gpio.GPIO_Pin = GPIO_Pin_9;                                                   // Chon chan PA9 lam TX
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;                                             // Cau hinh PA9 ngo ra alternate function push-pull
    gpio.GPIO_Speed = GPIO_Speed_50MHz;                                           // Chon toc do GPIO 50 MHz
    GPIO_Init(GPIOA, &gpio);                                                      // Ap dung cau hinh PA9
    gpio.GPIO_Pin = GPIO_Pin_10;                                                  // Chon chan PA10 lam RX
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;                                       // Cau hinh PA10 lam ngo vao floating
    GPIO_Init(GPIOA, &gpio);                                                      // Ap dung cau hinh PA10
    USART_StructInit(&usart);                                                     // Khoi tao cau hinh USART mac dinh
    usart.USART_BaudRate = UART_BAUDRATE;                                         // Dat toc do truyen 115200 baud
    usart.USART_WordLength = USART_WordLength_8b;                                 // Chon 8 bit du lieu
    usart.USART_StopBits = USART_StopBits_1;                                      // Chon 1 bit Stop
    usart.USART_Parity = USART_Parity_No;                                         // Khong su dung bit Parity
    usart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;                             // Cho phep ca truyen va nhan
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;             // Khong su dung dieu khien luong phan cung
    USART_Init(USART1, &usart);                                                   // Ap dung cau hinh USART1
    USART_Cmd(USART1, ENABLE);                                                    // Bat USART1
}

static void USART1_SendString(const char *str)                       // Ham gui chuoi ky tu qua USART1
{                                                                    // Bat dau ham USART1_SendString
    while (*str != '\0')                                             // Lap den khi gap ky tu ket thuc chuoi
    {                                                                // Bat dau vong lap gui chuoi
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) // Cho thanh ghi truyen trong
        {                                                            // Bat dau vong cho TXE
        } // Ket thuc vong cho TXE
        USART_SendData(USART1, (uint16_t)*str); // Gui mot ky tu qua USART1
        str++;                                  // Chuyen den ky tu tiep theo
    }
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) // Cho bit cuoi cung truyen xong
    {                                                           // Bat dau vong cho TC
    }
}

static void GPIO_Button_Init(void)                                              // Ham khoi tao GPIO nut nhan PB12
{                                                                               // Bat dau ham GPIO_Button_Init
    GPIO_InitTypeDef gpio;                                                      // Bien cau hinh GPIO
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE); // Cap clock cho GPIOB va AFIO
    GPIO_StructInit(&gpio);                                                     // Khoi tao cau hinh GPIO mac dinh
    gpio.GPIO_Pin = BUTTON_PIN;                                                 // Chon chan PB12
    gpio.GPIO_Mode = GPIO_Mode_IPU;                                             // Cau hinh ngo vao keo len noi
    gpio.GPIO_Speed = GPIO_Speed_2MHz;                                          // Chon toc do GPIO
    GPIO_Init(BUTTON_PORT, &gpio);                                              // Ap dung cau hinh cho PB12
}

static void EXTI_Button_Init(void)                               // Ham khoi tao EXTI12
{                                                                // Bat dau ham EXTI_Button_Init
    EXTI_InitTypeDef exti;                                       // Bien cau hinh EXTI
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource12); // Anh xa EXTI12 den PB12
    EXTI_ClearITPendingBit(BUTTON_EXTI_LINE);                    // Xoa co ngat ton dong
    EXTI_StructInit(&exti);                                      // Khoi tao cau hinh EXTI mac dinh
    exti.EXTI_Line = BUTTON_EXTI_LINE;                           // Chon duong ngat EXTI12
    exti.EXTI_Mode = EXTI_Mode_Interrupt;                        // Chon che do ngat
    exti.EXTI_Trigger = EXTI_Trigger_Falling;                    // Ngat khi tin hieu chuyen tu cao xuong thap
    exti.EXTI_LineCmd = DISABLE;                                 // Tam thoi khoa duong ngat EXTI12
    EXTI_Init(&exti);                                            // Ap dung cau hinh EXTI12
}

static void NVIC_Button_Init(void)              // Ham khoi tao NVIC cho EXTI15_10
{                                               // Bat dau ham NVIC_Button_Init
    NVIC_InitTypeDef nvic;                      // Bien cau hinh NVIC
    nvic.NVIC_IRQChannel = BUTTON_EXTI_IRQ;     // Chon kenh ngat EXTI15_10
    nvic.NVIC_IRQChannelPreemptionPriority = 5; // Dat do uu tien ngat truoc bang 5
    nvic.NVIC_IRQChannelSubPriority = 0;        // Dat do uu tien phu bang 0
    nvic.NVIC_IRQChannelCmd = ENABLE;           // Cho phep kenh ngat trong NVIC
    NVIC_Init(&nvic);                           // Ap dung cau hinh NVIC
}
void EXTI15_10_IRQHandler(void)                                                                       // Ham phuc vu ngat EXTI10 den EXTI15
{                                                                                                     // Bat dau ISR
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;                                                    // Co yeu cau chuyen Task sau ISR
    uint32_t count;                                                                                   // Bien tam luu so lan nhan nut
    TickType_t currentTick;                                                                           // Bien luu tick hien tai
    if (EXTI_GetITStatus(BUTTON_EXTI_LINE) != RESET)                                                  // Kiem tra co ngat EXTI12
    {                                                                                                 // Bat dau xu ly ngat EXTI12
        currentTick = xTaskGetTickCountFromISR();                                                     // Lay tick FreeRTOS trong ISR
        if (firstPress || ((TickType_t)(currentTick - lastButtonTick) >= pdMS_TO_TICKS(DEBOUNCE_MS))) // Kiem tra lan nhan dau tien hoac da qua thoi gian chong doi
        {                                                                                             // Bat dau xu ly lan nhan hop le
            firstPress = 0;                                                                           // Danh dau da xu ly lan nhan dau tien
            lastButtonTick = currentTick;                                                             // Cap nhat thoi diem nhan nut
            pressCount++;                                                                             // Tang bo dem so lan nhan nut
            count = pressCount;                                                                       // Sao chep bo dem vao bien tam
            if (CountQueue != NULL)                                                                   // Kiem tra Queue da duoc tao
            {                                                                                         // Bat dau gui du lieu vao Queue
                xQueueSendFromISR(CountQueue, &count, &xHigherPriorityTaskWoken);                     // Gui so lan nhan nut vao Queue tu ISR
            }
        }
        EXTI_ClearITPendingBit(BUTTON_EXTI_LINE);     // Xoa co ngat EXTI12
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken); // Chuyen Task neu can khi thoat ISR
    }
}

static void TaskUART(void *argument)                                                                      // Task nhan so dem tu Queue va gui UART
{                                                                                                         // Bat dau TaskUART
    uint32_t receivedCount;                                                                               // Bien nhan so lan nhan nut tu Queue
    char message[64];                                                                                     // Bo dem dinh dang chuoi gui UART
    (void)argument;                                                                                       // Danh dau tham so khong su dung
    USART1_SendString("== Nhom 4 ==\r\n");                                                                // Gui thong bao ten nhom
    USART1_SendString("Nhan nut tai PB12\r\n");                                                           // Gui huong dan nhan nut
    EXTI_ClearITPendingBit(BUTTON_EXTI_LINE);                                                             // Xoa co ngat truoc khi mo duong ngat
    EXTI->IMR |= BUTTON_EXTI_LINE;                                                                        // Mo duong ngat EXTI12
    for (;;)                                                                                              // Vong lap chay lien tuc cua Task
    {                                                                                                     // Bat dau vong lap Task
        if (xQueueReceive(CountQueue, &receivedCount, portMAX_DELAY) == pdPASS)                           // Cho den khi Queue co du lieu
        {                                                                                                 // Bat dau xu ly du lieu nhan duoc
            snprintf(message, sizeof(message), "So lan nhan nut: %lu\r\n", (unsigned long)receivedCount); // Dinh dang so dem thanh chuoi
            USART1_SendString(message);                                                                   // Gui chuoi ket qua qua UART1
        }
    }
}

int main(void)                                                 // Ham chinh chuong trinh
{                                                              // Bat dau ham main
    SystemInit();                                              // Khoi tao clock he thong theo cau hinh startup
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);            // Cau hinh nhom uu tien NVIC
    USART1_Init();                                             // Khoi tao USART1
    GPIO_Button_Init();                                        // Khoi tao GPIO nut nhan PB12
    EXTI_Button_Init();                                        // Khoi tao EXTI12 nhung chua mo duong ngat
    CountQueue = xQueueCreate(QUEUE_LENGTH, sizeof(uint32_t)); // Tao Queue gom 10 phan tu uint32_t
    if (CountQueue == NULL)                                    // Kiem tra tao Queue thanh cong
    {                                                          // Bat dau xu ly loi tao Queue
        while (1)                                              // Dung chuong trinh neu tao Queue that bai
        {                                                      // Bat dau vong lap dung
        }
    }
    if (xTaskCreate(TaskUART, "UART", 256, NULL, 3, NULL) != pdPASS) // Tao TaskUART voi stack depth 256 va uu tien 3
    {                                                                // Bat dau xu ly loi tao Task
        while (1)                                                    // Dung chuong trinh neu tao Task that bai
        {                                                            // Bat dau vong lap dung
        }
    }
    NVIC_Button_Init();    // Cau hinh va bat kenh ngat NVIC
    vTaskStartScheduler(); // Khoi dong bo lap lich FreeRTOS
    while (1)
    {
    }
}
