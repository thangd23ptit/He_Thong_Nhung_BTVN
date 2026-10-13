#include "stm32f10x.h"
#include "delay.h"
#include "spi.h"
#include "tft.h"

int main(void) {
    SystemClock_Config();       // 72MHz
    SPI1_Init();                // SPI1 master 18MHz, mode 3
    TFT_Init();                 // Khởi tạo ST7789 240x240

    // Xóa màn hình (đen)
    TFT_FillScreen(0x0000);

    // Thanh tiêu đề (nền xanh dương 0x001F, chữ trắng 0xFFFF)
    TFT_FillRect(0, 0, 240, 32, 0x001F);
    TFT_DrawString(24, 8, "SPI + TFT ST7789", 0xFFFF, 0x001F, 2);

    // Thông tin (bạn thay bằng thông tin của mình)
    TFT_DrawString(10,  50, "STM32F103C8T6",        0xFFE0, 0x0000, 2);
    TFT_DrawString(10,  76, "SPI1 - Register level", 0x07FF, 0x0000, 2);
    TFT_DrawString(10, 102, "Lop: LopHTN",          0xFFFF, 0x0000, 2);
    TFT_DrawString(10, 128, "Nhom: Nhom02",         0xFFFF, 0x0000, 2);

    // 7 thanh màu: đỏ, cam, vàng, xanh lá, cyan, xanh dương, tím
    uint16_t colors[7] = {0xF800, 0xFD20, 0xFFE0, 0x07E0, 0x07FF, 0x001F, 0xF81F};
    for (uint8_t i = 0; i < 7; i++) {
        TFT_FillRect(10 + i * 30, 165, 28, 40, colors[i]);
    }

    // Khung viền trắng
    TFT_DrawRect(0, 0, 240, 240, 0xFFFF);

    // Bộ đếm chạy để thấy màn hình đang hoạt động
    uint32_t cnt = 0;
    char buf[12];
    while (1) {
        uint32_t v = cnt++;
        int i = 10;
        buf[i] = '\0';
        do { buf[--i] = '0' + (v % 10); v /= 10; } while (v && i > 0);

        TFT_DrawString(10, 215, "Count: ", 0xFFFF, 0x0000, 2);
        TFT_DrawString(94, 215, &buf[i], 0x07E0, 0x0000, 2);
        delay_ms(500);
    }
}
