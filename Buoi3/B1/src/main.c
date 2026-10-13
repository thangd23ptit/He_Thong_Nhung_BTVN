#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"
#include "uart.h"
#include "ds1307.h"

void SystemClock_Config(void)
{
    /* 1. Bật HSE (External 8MHz crystal) */
    RCC->CR |= (1 << 16); // HSEON
    while (!(RCC->CR & (1 << 17))); // Chờ HSERDY

    /* 2. Cấu hình Flash latency (bắt buộc khi SYSCLK > 48MHz) */
    FLASH->ACR |= (1 << 4);   // PRFTBE: Prefetch buffer enable
    FLASH->ACR &= ~(0x7);     // Xóa LATENCY
    FLASH->ACR |= (0x2);      // 2 wait state (cho 48MHz < SYSCLK <= 72MHz)

    /* 3. Cấu hình prescaler: AHB=1, APB1=2 (max 36MHz), APB2=1 (max 72MHz) */
    RCC->CFGR &= ~(0xF << 4);
    RCC->CFGR |=  (0x0 << 4);   // HPRE = 0 => AHB = SYSCLK/1

    RCC->CFGR &= ~(0x7 << 8);
    RCC->CFGR |=  (0x4 << 8);   // PPRE1 = 100 => APB1 = HCLK/2 (36MHz)

    RCC->CFGR &= ~(0x7 << 11);
    RCC->CFGR |=  (0x0 << 11);  // PPRE2 = 0 => APB2 = HCLK/1 (72MHz)

    /* 4. Cấu hình PLL: nguồn = HSE, hệ số nhân = 9 => 8MHz x 9 = 72MHz */
    RCC->CFGR &= ~(1 << 16);    // PLLSRC = 1: chọn HSE làm nguồn cho PLL
    RCC->CFGR |=  (1 << 16);
    RCC->CFGR &= ~(1 << 17);    // PLLXTPRE = 0: HSE không chia (HSE/1)

    RCC->CFGR &= ~(0xF << 18);  // Xóa PLLMUL
    RCC->CFGR |=  (0x7 << 18);  // PLLMUL = 0111 => x9

    /* 5. Bật PLL và chờ khóa (lock) */
    RCC->CR |= (1 << 24);       // PLLON
    while (!(RCC->CR & (1 << 25))); // Chờ PLLRDY

    /* 6. Chuyển SYSCLK sang dùng PLL */
    RCC->CFGR &= ~(0x3);
    RCC->CFGR |=  (0x2);        // SW = 10: PLL làm nguồn SYSCLK
    while (((RCC->CFGR >> 2) & 0x3) != 0x2); // Chờ SWS xác nhận đã chuyển
}


// Delay đơn giản dùng vòng lặp
void Delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms * 4000; i++)
    {
        __NOP();
    }
}

// Chuyển số nguyên thành chuỗi 2 chữ số (VD: 9 -> "09")
void NumberToString2Digits(uint8_t num, char *buf)
{
    buf[0] = (num / 10) + '0';
    buf[1] = (num % 10) + '0';
    buf[2] = '\0';
}

// Chẩn đoán lỗi I2C: in mã lỗi, mức SCL/SDA và quét bus tìm thiết bị
static void I2C_Diagnose(void)
{
    char msg[96];
    uint8_t err   = I2C_GetLastError();
    uint8_t lines = I2C1_GetLineState();
    uint8_t scl   = lines & 1;
    uint8_t sda   = (lines >> 1) & 1;
    uint8_t found = 0;

    sprintf(msg, "  -> Ma loi=%d | SCL(PB6)=%d SDA(PB7)=%d (luc ranh phai la 1,1)\r\n", err, scl, sda);
    UART1_SendString(msg);

    if (!scl || !sda)
    {
        UART1_SendString("  -> SCL/SDA dang o muc 0: THIEU dien tro keo len (pull-up), day cham/chap, hoac module chua co nguon 5V\r\n");
    }

    // Quét địa chỉ 0x08..0x77
    UART1_SendString("  -> Quet I2C:");
    for (uint8_t addr = 0x08; addr <= 0x77; addr++)
    {
        uint8_t r = I2C_Probe(I2C1, addr);
        if (r == 2)
        {
            UART1_SendString(" [BUS LOI/KET - khong phat duoc START]");
            found = 0xFF;
            break;
        }
        if (r == 1)
        {
            sprintf(msg, " 0x%02X", addr);
            UART1_SendString(msg);
            found = 1;
        }
    }
    if (found == 0) UART1_SendString(" khong thay thiet bi nao (kiem tra nguon + day SDA/SCL)");
    UART1_SendString("\r\n");
}

// Xử lý lệnh nhận được từ UART để đặt lại thời gian
void Process_UART_Command(char *cmd, DS1307_Time *time)
{
    // Kiểm tra tiền tố lệnh bắt đầu bằng 'S' (Set)
    if (cmd[0] == 'S' || cmd[0] == 's')
    {
        int h, m, s, d, mo, y;
        // Phân tích định dạng: S HH MM SS DD MM YY
        if (sscanf(cmd + 1, "%d %d %d %d %d %d", &h, &m, &s, &d, &mo, &y) == 6)
        {
            time->hour  = (uint8_t)h;
            time->min   = (uint8_t)m;
            time->sec   = (uint8_t)s;
            time->date  = (uint8_t)d;
            time->month = (uint8_t)mo;
            time->year  = (uint8_t)y;
            time->day   = 1; // Mặc định

            DS1307_SetTime(time);
            UART1_SendString("\r\n[OK] Thiet lap thoi gian moi thanh cong!\r\n");
        }
        else
        {
            UART1_SendString("\r\n[Loi] Sai dinh dang! Vi du dung: S 14 30 00 25 09 24\r\n");
        }
    }
}

int main(void)
{ 
     SystemClock_Config();
    DS1307_Time currentTime;
    char buffer[100];
    char rxBuffer[50];
    uint8_t rxIndex = 0;

    // 1. Khởi tạo ngoại vi UART1 (9600 bps)
    UART1_Init();
    
    // 2. Khởi tạo RTC DS1307 qua I2C1
    DS1307_Init();

    UART1_SendString("\r\n====== HE THONG DS1307 RTC - STM32F103 ======\r\n");
    UART1_SendString("Cu phap cai dat: S HH MM SS DD MM YY\r\n\r\n");

    while (1)
    {
        // 1. Kiểm tra và đọc lệnh từ UART nếu có
        while (UART1_IsDataAvailable())
        {
            char c = UART1_ReceiveChar();
            UART1_SendChar(c); // Echo ký tự gõ lên terminal

            if (c == '\r' || c == '\n')
            {
                rxBuffer[rxIndex] = '\0';
                if (rxIndex > 0)
                {
                    Process_UART_Command(rxBuffer, &currentTime);
                    rxIndex = 0;
                }
            }
            else if (rxIndex < sizeof(rxBuffer) - 1)
            {
                rxBuffer[rxIndex++] = c;
            }
        }

        // 2. Đọc thời gian từ DS1307
        DS1307_GetTime(&currentTime);

        if (I2C_GetLastError() != 0)
        {
            UART1_SendString("[LOI I2C] Khong nhan ACK/du lieu tu DS1307\r\n");
            I2C_Diagnose();
            // Khởi tạo lại I2C1: hàm này sẽ tự chạy bus-recovery (bit-bang
            // giải phóng SDA/SCL) trước khi cấu hình lại phần cứng I2C, nhờ
            // đó bus tự phục hồi mà KHÔNG cần bấm nút Reset cứng nữa.
            DS1307_Init();
            Delay_ms(1000);
            continue;
        }

        // 2b. Nếu đồng hồ đang dừng (bit CH = 1, thường gặp khi mới cấp nguồn),
        //     tự khởi động bằng cách ghi giờ = thời điểm biên dịch (bit CH được xóa khi ghi)
        if (DS1307_IsHalted())
        {
            DS1307_GetBuildTime(&currentTime);
            DS1307_SetTime(&currentTime);
            UART1_SendString("[INFO] DS1307 dang dung (CH=1) -> da khoi dong, dat gio = luc bien dich.\r\n");
            UART1_SendString("       Gio chinh xac: gui lenh S HH MM SS DD MM YY\r\n");
            continue;
        }

        // 3. Format hiển thị: "TIME: HH:MM:SS - DATE: DD/MM/20YY\r\n"
        char strTmp[3];

        UART1_SendString("TIME: ");
        NumberToString2Digits(currentTime.hour, strTmp); UART1_SendString(strTmp); UART1_SendString(":");
        NumberToString2Digits(currentTime.min, strTmp);  UART1_SendString(strTmp); UART1_SendString(":");
        NumberToString2Digits(currentTime.sec, strTmp);  UART1_SendString(strTmp);

        UART1_SendString(" - DATE: ");
        NumberToString2Digits(currentTime.date, strTmp);  UART1_SendString(strTmp); UART1_SendString("/");
        NumberToString2Digits(currentTime.month, strTmp); UART1_SendString(strTmp); UART1_SendString("/20");
        NumberToString2Digits(currentTime.year, strTmp);  UART1_SendString(strTmp); UART1_SendString("\r\n");

        // Delay ~1 giây trước khi cập nhật lại màn hình
        Delay_ms(1000);
    }
}