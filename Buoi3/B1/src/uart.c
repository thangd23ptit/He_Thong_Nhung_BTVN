#include "uart.h"
#include "stm32f10x.h"

void UART1_Init(void)
{
    /* 1. Cấp xung Clock cho USART1 và GPIOA (đều nằm trên APB2) */
    RCC->APB2ENR |= (1 << 14); // Bit 14: USART1EN
    RCC->APB2ENR |= (1 << 2);  // Bit 2:  IOPAEN
    RCC->APB2ENR |= (1 << 0);  // Bit 0:  AFIOEN (Alternate Function)

    /* 2. Cấu hình chân TX: PA9 (Alternate Function Output Push-Pull, Max 50MHz)
     *    MODE9 = 11 (50MHz Output), CNF9 = 10 (AF Push-Pull) => 0xB
     *    PA9 thuộc thanh ghi CRH (từ bit 4 đến bit 7)
     */
    GPIOA->CRH &= ~(0xF << 4); // Xóa cấu hình cũ của PA9
    GPIOA->CRH |=  (0xB << 4); // Gán giá trị 0xB (1011b)

    /* 3. Cấu hình chân RX: PA10 (Input Floating)
     *    MODE10 = 00 (Input), CNF10 = 01 (Floating input) => 0x4
     *    PA10 thuộc thanh ghi CRH (từ bit 8 đến bit 11)
     */
    GPIOA->CRH &= ~(0xF << 8); // Xóa cấu hình cũ của PA10
    GPIOA->CRH |=  (0x4 << 8); // Gán giá trị 0x4 (0100b)

    /* 4. Cấu hình Baudrate = 9600 bps
     *    USART1 dùng xung nhịp PCLK2 = 72MHz:
     *    USARTDIV = 72,000,000 / (16 * 9600) = 468.75
     *    - Phần nguyên: 468 = 0x1D4
     *    - Phần thập phân: 0.75 * 16 = 12 = 0x0C
     *    => BRR = 0x1D4C
     */
    USART1->BRR = 0x1D4C;

    /* 5. Cấu hình khung truyền và bật USART1 (CR1, CR2, CR3)
     *    Bit 13 (UE): Bật bộ USART1 (USART Enable)
     *    Bit 3  (TE): Bật bộ truyền (Transmitter Enable)
     *    Bit 2  (RE): Bật bộ nhận (Receiver Enable)
     *    Mặc định: Word length 8 bit, 1 stop bit, không kiểm tra Parity.
     */
    USART1->CR1 |= (1 << 13) | (1 << 3) | (1 << 2);
    USART1->CR2 = 0x0000; // 1 Stop bit
    USART1->CR3 = 0x0000; // Không dùng phần cứng điều khiển luồng (No flow control)
}

void UART1_SendChar(char c)
{
    /* Chờ cho đến khi cờ TXE (Transmit data register empty - bit 7 của SR) lên 1 */
    while (!(USART1->SR & (1 << 7)));
    
    /* Ghi dữ liệu vào thanh ghi DR */
    USART1->DR = (uint16_t)c;
}

void UART1_SendString(const char *str)
{
    while (*str != '\0')
    {
        UART1_SendChar(*str);
        str++;
    }
}

uint8_t UART1_IsDataAvailable(void)
{
    /* Kiểm tra cờ RXNE (Read data register not empty - bit 5 của SR) */
    return (USART1->SR & (1 << 5)) ? 1 : 0;
}

char UART1_ReceiveChar(void)
{
    /* Chờ cho tới khi có dữ liệu đến (cờ RXNE = 1) */
    while (!(USART1->SR & (1 << 5)));
    
    /* Đọc ký tự từ thanh ghi DR (đồng thời tự động xóa cờ RXNE) */
    return (char)(USART1->DR & 0xFF);
}