/*====================================================
 * THU VIEN SU DUNG
 *====================================================*/

#include "stm32f10x.h"      // Thu vien STM32F10x Standard Peripheral Library
#include "FreeRTOS.h"        // Cac khai bao cot loi cua FreeRTOS
#include "task.h"            // Cac ham quan ly Task: xTaskCreate, vTaskDelay...
#include "queue.h"           // Cac ham quan ly Queue: xQueueCreate, xQueueSend...
#include <stdint.h>          // Cac kieu du lieu uint8_t, uint16_t, uint32_t

/*====================================================
 * 1. KHAI BAO CAU HINH
 *====================================================*/

/* Chu ky PWM co 1000 buoc: bo dem dem tu 0 den 999 */
#define PWM_PERIOD             999U

/* Moi lan nhan nut UP hoac DOWN, do sang thay doi 10% */
#define PWM_STEP               10U

/* TaskButton doc trang thai cac nut moi 10 ms */
#define BUTTON_DELAY_MS        10U

/* Yeu cau trang thai nut on dinh trong khoang 30 ms */
#define DEBOUNCE_MS             30U

/* So lan lay mau lien tiep can thiet de xac nhan trang thai */
#define DEBOUNCE_COUNT         (DEBOUNCE_MS / BUTTON_DELAY_MS)

/* Nut SELECT noi voi chan PB12 */
#define BTN_SELECT             GPIO_Pin_12

/* Nut UP noi voi chan PB13 */
#define BTN_UP                 GPIO_Pin_13

/* Nut DOWN noi voi chan PB15 */
#define BTN_DOWN               GPIO_Pin_15

/* Ma su kien: chon LED */
#define EVT_SELECT             0U

/* Ma su kien: tang do sang */
#define EVT_UP                 1U

/* Ma su kien: giam do sang */
#define EVT_DOWN               2U

/* So luong LED duoc dieu khien */
#define LED_COUNT              3U

/*====================================================
 * 2. KHAI BAO CAU TRUC DU LIEU
 *====================================================*/

/* Cau truc luu thong tin mot su kien dieu khien LED */
typedef struct
{
    uint8_t type;             // Loai su kien: SELECT, UP hoac DOWN
    uint8_t led;              // LED muc tieu: 0, 1 hoac 2
} LEDEvent_t;                 // Ten kieu du lieu cau truc su kien

/* Queue nhan su kien cua Task LED1 */
static QueueHandle_t qLED1 = NULL;

/* Queue nhan su kien cua Task LED2 */
static QueueHandle_t qLED2 = NULL;

/* Queue nhan su kien cua Task LED3 */
static QueueHandle_t qLED3 = NULL;

/* LED dang duoc chon: 0 = LED1, 1 = LED2, 2 = LED3 */
static volatile uint8_t selectedLED = 0U;

/* Do sang ban dau cua LED1, LED2 va LED3 la 30% */
static uint8_t brightness[LED_COUNT] = {30U, 30U, 30U};

/*====================================================
 * 3. KHOI TAO GPIO CHO BA NUT NHAN
 *====================================================*/

static void Buttons_Init(void)
{
    GPIO_InitTypeDef gpio;    // Bien cau hinh GPIO theo SPL

    /* Bat clock cho ngoai vi GPIOB */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* Dua cac truong cau hinh GPIO ve gia tri mac dinh */
    GPIO_StructInit(&gpio);

    /* Chon ba chan PB12, PB13 va PB15 */
    gpio.GPIO_Pin = BTN_SELECT | BTN_UP | BTN_DOWN;

    /* Cau hinh ngo vao co dien tro keo len noi */
    gpio.GPIO_Mode = GPIO_Mode_IPU;

    /* Cau hinh toc do GPIO; khong phai tan so lay mau debounce */
    gpio.GPIO_Speed = GPIO_Speed_2MHz;

    /* Ghi cau hinh vao cac thanh ghi GPIOB */
    GPIO_Init(GPIOB, &gpio);
}

/*====================================================
 * 4. KHOI TAO PWM TIM3
 *====================================================*/

static void PWM_Init(void)
{
    GPIO_InitTypeDef gpio;           // Cau hinh chan GPIO
    TIM_TimeBaseInitTypeDef tim;     // Cau hinh bo dem timer
    TIM_OCInitTypeDef oc;            // Cau hinh kenh Output Compare/PWM

    /* Bat clock GPIOA va GPIOB */
    RCC_APB2PeriphClockCmd(
        RCC_APB2Periph_GPIOA |       // Clock cho GPIOA
        RCC_APB2Periph_GPIOB,        // Clock cho GPIOB
        ENABLE
    );

    /* Bat clock TIM3 tren bus APB1 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* Khoi tao cac truong GPIO voi gia tri mac dinh */
    GPIO_StructInit(&gpio);

    /* PA6 = TIM3_CH1 va PA7 = TIM3_CH2 */
    gpio.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;

    /* Cau hinh chan lam ngo ra Alternate Function Push-Pull */
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;

    /* Toc do ngo ra GPIO */
    gpio.GPIO_Speed = GPIO_Speed_50MHz;

    /* Ap dung cau hinh cho PA6 va PA7 */
    GPIO_Init(GPIOA, &gpio);

    /* PB0 = TIM3_CH3 */
    gpio.GPIO_Pin = GPIO_Pin_0;

    /* Ap dung cau hinh ngo ra PWM cho PB0 */
    GPIO_Init(GPIOB, &gpio);

    /* Khoi tao cau hinh timer voi cac gia tri mac dinh */
    TIM_TimeBaseStructInit(&tim);

    /*
     * Gia su clock TIM3 = 72 MHz.
     * PSC = 71 => clock bo dem = 72 MHz / 72 = 1 MHz.
     */
    tim.TIM_Prescaler = 71U;

    /* ARR = 999 => moi chu ky co 1000 buoc dem */
    tim.TIM_Period = PWM_PERIOD;

    /* Bo dem tang tu 0 den ARR */
    tim.TIM_CounterMode = TIM_CounterMode_Up;

    /* Khong chia clock timer them */
    tim.TIM_ClockDivision = TIM_CKD_DIV1;

    /* Ghi cau hinh bo dem vao TIM3 */
    TIM_TimeBaseInit(TIM3, &tim);

    /* Khoi tao cau hinh cac kenh PWM */
    TIM_OCStructInit(&oc);

    /* Chon che do PWM mode 1 */
    oc.TIM_OCMode = TIM_OCMode_PWM1;

    /* Cho phep dau ra cua kenh PWM */
    oc.TIM_OutputState = TIM_OutputState_Enable;

    /* Dau ra PWM tich cuc muc cao */
    oc.TIM_OCPolarity = TIM_OCPolarity_High;

    /* Gia tri compare ban dau bang 0 */
    oc.TIM_Pulse = 0U;

    /* Khoi tao kenh PWM 1 cho LED1 */
    TIM_OC1Init(TIM3, &oc);

    /* Khoi tao kenh PWM 2 cho LED2 */
    TIM_OC2Init(TIM3, &oc);

    /* Khoi tao kenh PWM 3 cho LED3 */
    TIM_OC3Init(TIM3, &oc);

    /* Cho phep preload thanh ghi compare kenh 1 */
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);

    /* Cho phep preload thanh ghi compare kenh 2 */
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);

    /* Cho phep preload thanh ghi compare kenh 3 */
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);

    /* Cho phep preload thanh ghi ARR */
    TIM_ARRPreloadConfig(TIM3, ENABLE);

    /*
     * ARR + 1 = 1000.
     * Duty cycle (%) = CCR / 1000 * 100.
     * Do sang 30% tuong ung CCR = 300.
     */
    TIM_SetCompare1(TIM3, (uint16_t)(brightness[0] * 10U));

    /* Dat duty cycle ban dau cua LED2 la 30% */
    TIM_SetCompare2(TIM3, (uint16_t)(brightness[1] * 10U));

    /* Dat duty cycle ban dau cua LED3 la 30% */
    TIM_SetCompare3(TIM3, (uint16_t)(brightness[2] * 10U));

    /* Bat TIM3 de bat dau tao tin hieu PWM */
    TIM_Cmd(TIM3, ENABLE);
}

/*====================================================
 * 5. GUI SU KIEN DEN BA QUEUE
 *====================================================*/

static void BroadcastEvent(const LEDEvent_t *event)
{
    /* Gui su kien vao Queue cua LED1; cho neu Queue day */
    (void)xQueueSend(qLED1, event, portMAX_DELAY);

    /* Gui cung su kien vao Queue cua LED2 */
    (void)xQueueSend(qLED2, event, portMAX_DELAY);

    /* Gui cung su kien vao Queue cua LED3 */
    (void)xQueueSend(qLED3, event, portMAX_DELAY);
}

/*====================================================
 * 6. TASK DOC NUT NHAN VA CHONG DOI
 *====================================================*/

static void TaskButton(void *argument)
{
    /* Trang thai lay mau cua SELECT o lan doc truoc */
    uint8_t lastSelect = 1U;

    /* Trang thai lay mau cua UP o lan doc truoc */
    uint8_t lastUp = 1U;

    /* Trang thai lay mau cua DOWN o lan doc truoc */
    uint8_t lastDown = 1U;

    /* Trang thai SELECT da duoc xac nhan on dinh */
    uint8_t stableSelect = 1U;

    /* Trang thai UP da duoc xac nhan on dinh */
    uint8_t stableUp = 1U;

    /* Trang thai DOWN da duoc xac nhan on dinh */
    uint8_t stableDown = 1U;

    /* Bo dem so lan lay mau on dinh cua SELECT */
    uint8_t countSelect = 0U;

    /* Bo dem so lan lay mau on dinh cua UP */
    uint8_t countUp = 0U;

    /* Bo dem so lan lay mau on dinh cua DOWN */
    uint8_t countDown = 0U;

    /* Bien luu su kien truoc khi gui vao Queue */
    LEDEvent_t event;

    /* Danh dau tham so argument khong duoc su dung */
    (void)argument;

    /* Task chay lap lai cho den khi he thong dung */
    for (;;)
    {
        /* Doc muc logic thuc te tai chan PB12 */
        uint8_t rawSelect =
            GPIO_ReadInputDataBit(GPIOB, BTN_SELECT);

        /* Doc muc logic thuc te tai chan PB13 */
        uint8_t rawUp =
            GPIO_ReadInputDataBit(GPIOB, BTN_UP);

        /* Doc muc logic thuc te tai chan PB15 */
        uint8_t rawDown =
            GPIO_ReadInputDataBit(GPIOB, BTN_DOWN);

        /*--------------------------------------------
         * DEBOUNCE NUT SELECT
         *--------------------------------------------*/

        /* Kiem tra trang thai SELECT co giong lan doc truoc */
        if (rawSelect == lastSelect)
        {
            /* Tang bo dem neu chua dat nguong */
            if (countSelect < DEBOUNCE_COUNT)
            {
                countSelect++;
            }
        }
        else
        {
            /* Trang thai thay doi, dat lai bo dem */
            countSelect = 0U;
        }

        /* Luu trang thai SELECT hien tai cho lan doc sau */
        lastSelect = rawSelect;

        /*
         * Chi chap nhan trang thai moi neu:
         * - Bo dem da dat nguong debounce.
         * - Trang thai moi khac trang thai da xac nhan.
         */
        if ((countSelect >= DEBOUNCE_COUNT) &&
            (stableSelect != rawSelect))
        {
            /* Cap nhat trang thai SELECT da xac nhan */
            stableSelect = rawSelect;

            /* Chi tao su kien khi nut duoc nhan, muc logic 0 */
            if (stableSelect == 0U)
            {
                /* Chuyen LED duoc chon theo thu tu 0, 1, 2, 0 */
                selectedLED =
                    (uint8_t)((selectedLED + 1U) % LED_COUNT);

                /* Gan ma su kien chon LED */
                event.type = EVT_SELECT;

                /* Gan chi so LED vua duoc chon */
                event.led = selectedLED;

                /* Gui su kien den ba Task LED */
                BroadcastEvent(&event);
            }
        }

        /*--------------------------------------------
         * DEBOUNCE NUT UP
         *--------------------------------------------*/

        /* Kiem tra trang thai UP co giong lan doc truoc */
        if (rawUp == lastUp)
        {
            /* Tang bo dem den nguong debounce */
            if (countUp < DEBOUNCE_COUNT)
            {
                countUp++;
            }
        }
        else
        {
            /* Trang thai thay doi, bat dau dem lai */
            countUp = 0U;
        }

        /* Luu trang thai UP hien tai */
        lastUp = rawUp;

        /* Xac nhan trang thai UP sau nhieu lan lay mau */
        if ((countUp >= DEBOUNCE_COUNT) &&
            (stableUp != rawUp))
        {
            /* Cap nhat trang thai UP da xac nhan */
            stableUp = rawUp;

            /* Neu muc logic bang 0 thi nut UP dang duoc nhan */
            if (stableUp == 0U)
            {
                /* Tao su kien tang do sang */
                event.type = EVT_UP;

                /* Gan su kien cho LED dang duoc chon */
                event.led = selectedLED;

                /* Gui su kien den ba Queue */
                BroadcastEvent(&event);
            }
        }

        /*--------------------------------------------
         * DEBOUNCE NUT DOWN
         *--------------------------------------------*/

        /* Kiem tra trang thai DOWN co giong lan doc truoc */
        if (rawDown == lastDown)
        {
            /* Tang bo dem den nguong debounce */
            if (countDown < DEBOUNCE_COUNT)
            {
                countDown++;
            }
        }
        else
        {
            /* Trang thai thay doi, bat dau dem lai */
            countDown = 0U;
        }

        /* Luu trang thai DOWN hien tai */
        lastDown = rawDown;

        /* Xac nhan trang thai DOWN sau nhieu lan lay mau */
        if ((countDown >= DEBOUNCE_COUNT) &&
            (stableDown != rawDown))
        {
            /* Cap nhat trang thai DOWN da xac nhan */
            stableDown = rawDown;

            /* Chi xu ly khi nut DOWN duoc nhan */
            if (stableDown == 0U)
            {
                /* Tao su kien giam do sang */
                event.type = EVT_DOWN;

                /* Gan su kien cho LED dang duoc chon */
                event.led = selectedLED;

                /* Gui su kien den ba Queue */
                BroadcastEvent(&event);
            }
        }

        /*
         * Cho 10 ms truoc khi doc nut tiep theo.
         * Task khac co the su dung CPU trong khoang cho nay.
         */
        vTaskDelay(pdMS_TO_TICKS(BUTTON_DELAY_MS));
    }
}

/*====================================================
 * 7. TASK XU LY DO SANG LED
 *====================================================*/

static void LEDTask(void *argument)
{
    /* Chuyen tham so argument thanh chi so LED */
    uint32_t id = (uint32_t)argument;

    /* Con tro den Queue ma Task nay se doc */
    QueueHandle_t queue;

    /* Bien nhan su kien tu Queue */
    LEDEvent_t event;

    /* Chon Queue tuong ung voi LED1 */
    if (id == 0U)
    {
        queue = qLED1;
    }
    /* Chon Queue tuong ung voi LED2 */
    else if (id == 1U)
    {
        queue = qLED2;
    }
    /* Cac chi so con lai duoc gan vao Queue LED3 */
    else
    {
        queue = qLED3;
    }

    /* Vong lap xu ly su kien lien tuc */
    for (;;)
    {
        /*
         * Cho su kien tu Queue.
         * portMAX_DELAY cho phep Task cho vo han neu Queue rong.
         */
        if (xQueueReceive(
                queue,                 /* Queue can doc */
                &event,                /* Noi luu su kien nhan duoc */
                portMAX_DELAY)         /* Thoi gian cho */
            == pdPASS)                 /* Kiem tra nhan thanh cong */
        {
            /*
             * Chi xu ly su kien neu no danh cho LED cua Task nay.
             * id = 0: LED1; id = 1: LED2; id = 2: LED3.
             */
            if (event.led == id)
            {
                /*------------------------------------
                 * TANG DO SANG
                 *------------------------------------*/

                /* Kiem tra su kien co phai tang do sang */
                if (event.type == EVT_UP)
                {
                    /*
                     * Neu do sang chua vuot 100% sau khi tang,
                     * cong them PWM_STEP.
                     */
                    if (brightness[id] <= (100U - PWM_STEP))
                    {
                        brightness[id] += PWM_STEP;
                    }
                    else
                    {
                        /* Gioi han do sang toi da la 100% */
                        brightness[id] = 100U;
                    }
                }

                /*------------------------------------
                 * GIAM DO SANG
                 *------------------------------------*/

                /* Kiem tra su kien co phai giam do sang */
                else if (event.type == EVT_DOWN)
                {
                    /*
                     * Neu do sang du lon, tru di PWM_STEP.
                     */
                    if (brightness[id] >= PWM_STEP)
                    {
                        brightness[id] -= PWM_STEP;
                    }
                    else
                    {
                        /* Gioi han do sang toi thieu la 0% */
                        brightness[id] = 0U;
                    }
                }

                /*------------------------------------
                 * CAP NHAT PWM
                 *------------------------------------*/

                /*
                 * Chuyen phan tram do sang thanh gia tri CCR.
                 * Vi ARR + 1 = 1000 nen CCR = brightness * 10.
                 */

                /* Neu Task nay quan ly LED1 */
                if (id == 0U)
                {
                    /* Cap nhat duty cycle TIM3_CH1 tren PA6 */
                    TIM_SetCompare1(
                        TIM3,
                        (uint16_t)(brightness[id] * 10U)
                    );
                }
                /* Neu Task nay quan ly LED2 */
                else if (id == 1U)
                {
                    /* Cap nhat duty cycle TIM3_CH2 tren PA7 */
                    TIM_SetCompare2(
                        TIM3,
                        (uint16_t)(brightness[id] * 10U)
                    );
                }
                /* Neu Task nay quan ly LED3 */
                else if (id == 2U)
                {
                    /* Cap nhat duty cycle TIM3_CH3 tren PB0 */
                    TIM_SetCompare3(
                        TIM3,
                        (uint16_t)(brightness[id] * 10U)
                    );
                }
            }
        }
    }
}

/*====================================================
 * 8. CAC HAM KHOI TAO BA TASK LED
 *====================================================*/

/* Entry function cua Task dieu khien LED1 */
static void TaskLED1(void *argument)
{
    /* Khong su dung tham so dau vao cua Task */
    (void)argument;

    /* Goi ham xu ly LED chung voi id = 0 */
    LEDTask((void *)0U);
}

/* Entry function cua Task dieu khien LED2 */
static void TaskLED2(void *argument)
{
    /* Khong su dung tham so dau vao cua Task */
    (void)argument;

    /* Goi ham xu ly LED chung voi id = 1 */
    LEDTask((void *)1U);
}

/* Entry function cua Task dieu khien LED3 */
static void TaskLED3(void *argument)
{
    /* Khong su dung tham so dau vao cua Task */
    (void)argument;

    /* Goi ham xu ly LED chung voi id = 2 */
    LEDTask((void *)2U);
}

/*====================================================
 * 9. HAM MAIN
 *====================================================*/

int main(void)
{
    /* Khoi tao cau hinh clock he thong theo startup/SPL */
    SystemInit();

    /* Khoi tao TIM3 va ba kenh PWM dieu khien LED */
    PWM_Init();

    /* Khoi tao GPIO cho ba nut nhan */
    Buttons_Init();

    /* Tao Queue cho Task LED1, toi da 8 su kien */
    qLED1 = xQueueCreate(8U, sizeof(LEDEvent_t));

    /* Tao Queue cho Task LED2 */
    qLED2 = xQueueCreate(8U, sizeof(LEDEvent_t));

    /* Tao Queue cho Task LED3 */
    qLED3 = xQueueCreate(8U, sizeof(LEDEvent_t));

    /*
     * Neu mot Queue tao that bai, dung chuong trinh.
     * Khong duoc tiep tuc su dung Queue NULL.
     */
    if ((qLED1 == NULL) ||
        (qLED2 == NULL) ||
        (qLED3 == NULL))
    {
        while (1)
        {
            /* Dung tai day de bao loi khoi tao */
        }
    }

    /* Tao Task doc nut nhan, stack depth 256, uu tien 3 */
    if (xTaskCreate(
            TaskButton,               /* Ham chay cua Task */
            "Button",                 /* Ten Task */
            256,                      /* Kich thuoc stack */
            NULL,                     /* Khong truyen tham so */
            3,                        /* Muc uu tien */
            NULL)                     /* Khong luu Task handle */
        != pdPASS)                    /* Kiem tra tao Task thanh cong */
    {
        while (1)
        {
            /* Dung neu khong tao duoc TaskButton */
        }
    }

    /* Tao Task LED1, stack depth 128, uu tien 2 */
    if (xTaskCreate(
            TaskLED1,                 /* Ham chay cua Task LED1 */
            "LED1",                   /* Ten Task */
            128,                      /* Kich thuoc stack */
            NULL,                     /* Khong truyen tham so */
            2,                        /* Muc uu tien */
            NULL)                     /* Khong luu Task handle */
        != pdPASS)
    {
        while (1)
        {
            /* Dung neu khong tao duoc Task LED1 */
        }
    }

    /* Tao Task LED2, stack depth 128, uu tien 2 */
    if (xTaskCreate(
            TaskLED2,                 /* Ham chay cua Task LED2 */
            "LED2",                   /* Ten Task */
            128,                      /* Kich thuoc stack */
            NULL,                     /* Khong truyen tham so */
            2,                        /* Muc uu tien */
            NULL)                     /* Khong luu Task handle */
        != pdPASS)
    {
        while (1)
        {
            /* Dung neu khong tao duoc Task LED2 */
        }
    }

    /* Tao Task LED3, stack depth 128, uu tien 2 */
    if (xTaskCreate(
            TaskLED3,                 /* Ham chay cua Task LED3 */
            "LED3",                   /* Ten Task */
            128,                      /* Kich thuoc stack */
            NULL,                     /* Khong truyen tham so */
            2,                        /* Muc uu tien */
            NULL)                     /* Khong luu Task handle */
        != pdPASS)
    {
        while (1)
        {
            /* Dung neu khong tao duoc Task LED3 */
        }
    }

    /* Bat dau bo lap lich FreeRTOS */
    vTaskStartScheduler();

    /*
     * Neu chuong trinh chay den day, scheduler khong khoi dong.
     * Nguyen nhan co the la thieu heap hoac cau hinh port sai.
     */
    while (1)
    {
        /* Dung tai day de kiem tra loi scheduler */
    }
}