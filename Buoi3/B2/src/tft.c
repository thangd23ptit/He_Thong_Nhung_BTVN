#include "tft.h"
#include "spi.h"
#include "delay.h"
#include "font.h"

/* Ghi chú các chân điều khiển (ghi thẳng thanh ghi BSRR):
 *   BSRR bit 0..15  = đặt chân lên 1
 *   BSRR bit 16..31 = kéo chân xuống 0
 *   CS  = PA4  (GPIOA)
 *   DC  = PB0  (GPIOB)   0: lệnh, 1: dữ liệu
 *   RST = PB1  (GPIOB)
 *   BL  = PB10 (GPIOB)
 */

// Gửi 1 lệnh (DC = 0)
static void tft_cmd(uint8_t cmd) {
    SPI1_WaitDone();                  // Chờ byte trước gửi xong rồi mới đổi DC
    GPIOB->BSRR = (1 << 16);          // DC = 0 (PB0)
    GPIOA->BSRR = (1 << 20);          // CS = 0 (PA4)
    SPI1_Write(cmd);
    SPI1_WaitDone();
    GPIOA->BSRR = (1 << 4);           // CS = 1
}

// Gửi 1 byte dữ liệu (DC = 1)
static void tft_data8(uint8_t data) {
    GPIOB->BSRR = (1 << 0);           // DC = 1
    GPIOA->BSRR = (1 << 20);          // CS = 0
    SPI1_Write(data);
    SPI1_WaitDone();
    GPIOA->BSRR = (1 << 4);           // CS = 1
}

static void tft_gpio_init(void) {
    // Bật clock GPIOA (bit 2) và GPIOB (bit 3)
    RCC->APB2ENR |= (1 << 3) | (1 << 2);

    // PA4 (CS): output push-pull 50MHz (0x3)
    GPIOA->CRL &= ~(0xF << 16);
    GPIOA->CRL |=  (0x3 << 16);

    // PB0 (DC): output push-pull 50MHz
    GPIOB->CRL &= ~(0xF << 0);
    GPIOB->CRL |=  (0x3 << 0);

    // PB1 (RST): output push-pull 50MHz
    GPIOB->CRL &= ~(0xF << 4);
    GPIOB->CRL |=  (0x3 << 4);

    // PB10 (BL): output push-pull 50MHz (CRH, nibble thứ 2 => bit 8)
    GPIOB->CRH &= ~(0xF << 8);
    GPIOB->CRH |=  (0x3 << 8);

    // Trạng thái ban đầu: CS = 1, DC = 1, RST = 1
    GPIOA->BSRR = (1 << 4);
    GPIOB->BSRR = (1 << 0) | (1 << 1);
}

// Đặt cửa sổ ghi RAM: cột x0..x1, hàng y0..y1
static void tft_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    tft_cmd(0x2A);                                   // CASET: đặt cột
    tft_data8(x0 >> 8); tft_data8(x0 & 0xFF);
    tft_data8(x1 >> 8); tft_data8(x1 & 0xFF);

    tft_cmd(0x2B);                                   // RASET: đặt hàng
    tft_data8(y0 >> 8); tft_data8(y0 & 0xFF);
    tft_data8(y1 >> 8); tft_data8(y1 & 0xFF);

    tft_cmd(0x2C);                                   // RAMWR: bắt đầu ghi pixel
}

void TFT_Init(void) {
    tft_gpio_init();

    // Reset cứng: RST 1 -> 0 -> 1
    GPIOB->BSRR = (1 << 1);   delay_ms(10);
    GPIOB->BSRR = (1 << 17);  delay_ms(20);
    GPIOB->BSRR = (1 << 1);   delay_ms(150);

    tft_cmd(0x01); delay_ms(150);          // SWRESET
    tft_cmd(0x11); delay_ms(120);          // SLPOUT: thoát chế độ ngủ

    tft_cmd(0x3A); tft_data8(0x55);        // COLMOD: 16 bit/pixel (RGB565)
    tft_cmd(0x36); tft_data8(0x00);        // MADCTL: hướng mặc định, thứ tự RGB

    tft_cmd(0xB2); tft_data8(0x0C); tft_data8(0x0C); tft_data8(0x00);
                   tft_data8(0x33); tft_data8(0x33);   // Porch setting
    tft_cmd(0xB7); tft_data8(0x35);        // Gate control
    tft_cmd(0xBB); tft_data8(0x19);        // VCOM
    tft_cmd(0xC0); tft_data8(0x2C);        // LCM control
    tft_cmd(0xC2); tft_data8(0x01);        // VDV & VRH enable
    tft_cmd(0xC3); tft_data8(0x12);        // VRH
    tft_cmd(0xC4); tft_data8(0x20);        // VDV
    tft_cmd(0xC6); tft_data8(0x0F);        // Frame rate 60Hz
    tft_cmd(0xD0); tft_data8(0xA4); tft_data8(0xA1);   // Power control

    tft_cmd(0x21);                         // INVON: bật đảo màu (module IPS cần)
    tft_cmd(0x13);                         // NORON
    delay_ms(10);
    tft_cmd(0x29);                         // DISPON: bật hiển thị
    delay_ms(120);

    GPIOB->BSRR = (1 << 10);               // BL = 1: bật đèn nền (PB10)

    TFT_FillScreen(0x0000);
}

void TFT_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    if (x >= 240 || y >= 240 || w == 0 || h == 0) return;
    if (x + w > 240) w = 240 - x;
    if (y + h > 240) h = 240 - y;

    tft_set_window(x, y, x + w - 1, y + h - 1);

    uint8_t hi = color >> 8;
    uint8_t lo = color & 0xFF;
    uint32_t count = (uint32_t)w * h;

    GPIOB->BSRR = (1 << 0);                // DC = 1: dữ liệu
    GPIOA->BSRR = (1 << 20);               // CS = 0
    while (count--) {
        SPI1_Write(hi);
        SPI1_Write(lo);
    }
    SPI1_WaitDone();
    GPIOA->BSRR = (1 << 4);                // CS = 1
}

void TFT_FillScreen(uint16_t color) {
    TFT_FillRect(0, 0, 240, 240, color);
}

void TFT_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    TFT_FillRect(x, y, 1, 1, color);
}

void TFT_DrawHLine(uint16_t x, uint16_t y, uint16_t w, uint16_t color) {
    TFT_FillRect(x, y, w, 1, color);
}

void TFT_DrawVLine(uint16_t x, uint16_t y, uint16_t h, uint16_t color) {
    TFT_FillRect(x, y, 1, h, color);
}

void TFT_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    TFT_DrawHLine(x, y, w, color);
    TFT_DrawHLine(x, y + h - 1, w, color);
    TFT_DrawVLine(x, y, h, color);
    TFT_DrawVLine(x + w - 1, y, h, color);
}

void TFT_DrawChar(uint16_t x, uint16_t y, char c, uint16_t fg, uint16_t bg, uint8_t size) {
    if (c < 0x20 || c > 0x7E) c = '?';
    const uint8_t *glyph = font5x7[c - 0x20];

    uint16_t cw = 6 * size;                // 5 cột + 1 cột trống
    uint16_t ch = 8 * size;                // 7 hàng + 1 hàng trống
    if (x + cw > 240 || y + ch > 240) return;

    tft_set_window(x, y, x + cw - 1, y + ch - 1);
    GPIOB->BSRR = (1 << 0);                // DC = 1
    GPIOA->BSRR = (1 << 20);               // CS = 0

    for (uint8_t row = 0; row < 8; row++) {
        for (uint8_t sy = 0; sy < size; sy++) {
            for (uint8_t col = 0; col < 6; col++) {
                uint16_t px = bg;
                if (col < 5 && row < 7 && (glyph[col] & (1 << row))) px = fg;
                for (uint8_t sx = 0; sx < size; sx++) {
                    SPI1_Write(px >> 8);
                    SPI1_Write(px & 0xFF);
                }
            }
        }
    }
    SPI1_WaitDone();
    GPIOA->BSRR = (1 << 4);                // CS = 1
}

void TFT_DrawString(uint16_t x, uint16_t y, const char *s, uint16_t fg, uint16_t bg, uint8_t size) {
    while (*s) {
        if (x + 6 * size > 240) {          // Tự động xuống dòng
            x = 0;
            y += 8 * size;
        }
        TFT_DrawChar(x, y, *s++, fg, bg, size);
        x += 6 * size;
    }
}
