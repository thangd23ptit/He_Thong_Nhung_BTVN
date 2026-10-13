# Bài tập 04 - ADC + Timer + DMA + UART (STM32F103, thanh ghi)

## 1. Luồng hoạt động
```
TIM3 (100Hz) --TRGO--> ADC1 CH5 (PA5) --DMA1_CH1--> adc_buf[100] (circular)
                                                     |
                     ngắt Half-Transfer  (HT) -> nửa đầu  [0..49]  an toàn
                     ngắt Transfer-Complete (TC) -> nửa sau [50..99] an toàn
                                                     |
main: đổi số -> chuỗi "1234\n\r" -> UART1 + DMA1_CH4 (TX) -> PC
```
- 100 Hz x 1 s = 100 mẫu; mỗi nửa buffer 50 mẫu = 0.5 s -> cứ 0.5 s có 1 ngắt (HT rồi TC luân phiên).
- Timer: 72MHz / 7200 / 100 = 100Hz. ADC EXTSEL = 100 (TIM3_TRGO).
- UART1 115200-8N1. 300 byte gửi mất ~26ms << 500ms nên không bị chồng dữ liệu.

## 2. Lắp mạch
| Linh kiện | Chân STM32 |
|---|---|
| Biến trở chân giữa (wiper) | PA5 |
| Biến trở 2 chân ngoài | 3V3 và GND |
| USB-UART: RXD | PA9 (TX1) |
| USB-UART: GND | GND |
| (tuỳ chọn) USB-UART: TXD | PA10 |
| ST-Link | SWDIO, SWCLK, GND, 3V3 |

Lưu ý: cấp biến trở bằng 3V3 (không dùng 5V). Nối chung GND giữa USB-UART, ST-Link và board.

## 3. Cài công cụ (Ubuntu/Debian)
```
sudo apt update
sudo apt install gcc-arm-none-eabi make stlink-tools picocom python3-serial
sudo usermod -aG dialout $USER     # rồi đăng xuất/đăng nhập lại để dùng /dev/ttyUSB0
```

## 4. Build & nạp
```
make            # build
make flash      # nạp qua ST-Link (st-flash)
```

## 5. Test
1. Cắm USB-UART, kiểm tra cổng: `ls /dev/ttyUSB*` (hoặc /dev/ttyACM0).
2. Xem dữ liệu thô:
   ```
   picocom -b 115200 /dev/ttyUSB0        # thoát: Ctrl+A rồi Ctrl+X
   ```
   Xoay biến trở -> giá trị chạy trong khoảng 0..4095 (0V..3.3V).
3. Kiểm tra định lượng (đúng 100 mẫu/giây):
   ```
   python3 tools/test_uart.py /dev/ttyUSB0
   ```
   Kết quả mong đợi: `mau/giay ~ 100`, `loi=0`, min/max thay đổi khi xoay biến trở.
4. LED PC13 nháy mỗi 0.5s (mỗi ngắt HT/TC) -> đây là cách xem nhanh DMA + ngắt có chạy không.

## 6. Nếu lỗi
| Hiện tượng | Nguyên nhân thường gặp |
|---|---|
| Không có dữ liệu, LED không nháy | Timer chưa chạy / ADC chưa bật DMA. Kiểm tra `TIM3_Start()` và thứ tự init |
| LED nháy nhưng terminal trống | Sai baud, nhầm TX/RX, chưa nối GND, sai cổng /dev/tty* |
| Ký tự rác | Baud không khớp (phải 115200), hoặc HSE không phải 8MHz |
| Giá trị luôn 0 hoặc 4095 | Biến trở chưa nối đúng PA5 / 3V3 / GND |
| `Permission denied` cổng serial | Chưa thêm user vào nhóm `dialout` |
| `st-flash` không thấy thiết bị | Kiểm tra dây ST-Link, chạy `st-info --probe` |

## 7. Cấu trúc
```
src/main.c      vòng lặp chính, định dạng chuỗi, gửi UART
src/timer.c     TIM3 100Hz + TRGO
src/adc.c       ADC1 CH5, trigger TIM3_TRGO, bật DMA
src/dma.c       DMA1 CH1 (circular, HT/TC IRQ) + handler
src/uart.c      USART1 115200 + DMA1 CH4 (TX)   (từ code của bạn)
src/startup_stm32f103.s   thêm DMA1_Channel1_IRQHandler vào IRQ11
```
