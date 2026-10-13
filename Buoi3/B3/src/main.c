#include "stm32f10x.h"
#include "uart.h"
#include "button.h"
#include <stdio.h>
#include <string.h>


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



// Hàm tạo trễ dùng SysTick (do bạn cung cấp)
void delay_ms(int time) {
    while(time--) {
        SysTick->LOAD = 72000 - 1; // Hệ thống chạy 72MHz
        SysTick->VAL  = 0;
        SysTick->CTRL = 5;
        while(!(SysTick->CTRL & (1 << 16)));
    }
    SysTick->CTRL = 0; // Tắt SysTick sau khi dùng xong
}

int main(void) {
    SystemClock_Config();
    // Biến lưu trữ số lần nhấn nút
    uint32_t btn_count = 0;
    
    // Mảng (Buffer) lưu trữ chuỗi kí tự để DMA lấy gửi đi
    char tx_buffer[100]; 

    // Khởi tạo ngoại vi
    UART1_DMA_Init();
    Button_Init();
    Led_Init();

    while (1) {
        // Kiểm tra nếu nút được nhấn
        if (Button_IsPressed()) {
            Led_Set(1);
            delay_ms(20); // Delay 20ms để chống dội phím (Debounce)
            
            // Kiểm tra lại chắc chắn nút vẫn đang được nhấn
            if (Button_IsPressed()) {
                btn_count++; // Tăng giá trị lên 1 đơn vị
                
                // Format bản tin theo yêu cầu đề bài: <ID-Lớp><ID-Nhóm>:BTN:<Giá trị nút nhấn>\n\r
                // Bạn hãy thay "LopXYZ" và "Nhom01" thành thông tin thực tế của bạn
                sprintf(tx_buffer, "LopXYZ_Nhom01:BTN:%lu\n\r", btn_count);
                
                // CHÚ Ý QUAN TRỌNG: 
                // Kiểm tra xem DMA có đang bận gửi bản tin trước đó không.
                // Nếu CNDTR > 0 nghĩa là vẫn còn byte chưa gửi xong, cần đợi để không bị ghi đè chuỗi
                while (DMA1_Channel4->CNDTR > 0) {}

                // Yêu cầu DMA gửi bản tin (Không dùng hàm chờ của UART)
                UART1_Send_DMA((uint8_t*)tx_buffer, strlen(tx_buffer));
                
                // Chờ cho đến khi người dùng nhả nút ra (tránh việc nhấn giữ bị đếm liên tục)
                while (Button_IsPressed()) {}
                Led_Set(0);
                delay_ms(20); // Chống dội phím khi nhả
            }
        } else {
            Led_Set(0);
        }
    }
}