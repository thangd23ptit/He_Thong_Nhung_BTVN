#include "ADC.h"              // Include các định nghĩa/thanh ghi và hàm liên quan đến ADC
#include "RCC.h"              // Include các hàm điều khiển clock RCC

#define GPIOA_CRL *((volatile uint32_t *)0x40010800)
                              // Định nghĩa thanh ghi GPIOA_CRL
                              // Địa chỉ 0x40010800 là GPIOA_CRL trên STM32F103
                              // volatile: báo cho compiler biết giá trị thanh ghi có thể thay đổi bất kỳ lúc nào
                              // uint32_t: thanh ghi có kích thước 32 bit


void ADC01_CH3_Init(void)
{
    RCC_Enable_PortA();
    // Bật clock cho GPIOA
    // ADC Channel 3 tương ứng với chân PA3


    RCC_Enable_ADC01();
    // Bật clock cho ADC1
    // Hàm này thường sẽ bật clock ADC1 trong RCC_APB2ENR


    GPIOA_CRL &= ~(uint32_t)(0xF << 12);
    // Cấu hình PA3
    //
    

    ADC01_CR1 = 0;
    // Xóa toàn bộ ADC1_CR1
    // Đưa các thiết lập trong CR1 về mặc định


    ADC01_SMPR2 &= ~(uint32_t)(0x7 << 9);
    // Xóa 3 bit cấu hình thời gian lấy mẫu cho ADC Channel 3



    ADC01_SMPR2 |= (uint32_t)(0x2 << 9);
    // Thiết lập thời gian lấy mẫu cho Channel 3
    //
    // 0x2 = 010
    //
    // Theo STM32F103:
    // 000 = 1.5 cycles
    // 001 = 7.5 cycles
    // 010 = 13.5 cycles
    // 011 = 28.5 cycles
    // 100 = 41.5 cycles
    // 101 = 55.5 cycles
    // 110 = 71.5 cycles
    // 111 = 239.5 cycles
    //
    // Ở đây chọn 13.5 ADC cycles


    ADC01_SQR1 &= ~(uint32_t)(0xF << 20);
    // Xóa 4 bit L[3:0] trong ADC1_SQR1
    //
    // L[3:0] xác định số lượng conversion trong một sequence
    //
    // 0000 = 1 conversion
    //
    // Vì chỉ đọc Channel 3 nên sequence chỉ có 1 conversion


    ADC01_SQR3 &= ~(uint32_t)(0x1F);
    // Xóa 5 bit đầu tiên của ADC1_SQR3
    //
    // SQ1 nằm tại bit 0 -> 4
    // SQ1 xác định channel được chuyển đổi đầu tiên


    ADC01_SQR3 |= 3;
    // Đặt SQ1 = 3
    //
    // Sequence:
    // Conversion thứ 1 -> ADC Channel 3
    //
    // ADC Channel 3 tương ứng với chân PA3


    ADC01_CR2 = 0;
    // Xóa toàn bộ ADC1_CR2
    // Đưa ADC về trạng thái cấu hình ban đầu


    ADC01_CR2 |= (1 << 1);
    // Set bit 1 = CONT
    //
    // CONT = 1
    // ADC hoạt động ở chế độ continuous conversion
    //
    // Sau khi chuyển đổi xong Channel 3,
    // ADC sẽ tự động chuyển đổi lại liên tục


    ADC01_CR2 |= (1 << 0);
    // Set bit 0 = ADON
    //
    // ADON = 1
    // Bật ADC1


    for (volatile uint32_t i = 0; i < 10000; i++)
    {
    }
    // Delay một khoảng thời gian
    //
    // Mục đích:
    // Cho ADC ổn định sau khi bật ADC
    //
    // Đây là delay bằng vòng lặp,
    // thời gian thực tế phụ thuộc vào clock CPU


    ADC01_CR2 |= (1 << 3);
    // Set bit 3 = RSTCAL
    //
    // Yêu cầu ADC reset thanh ghi hiệu chuẩn
    // trước khi thực hiện calibration


    while (ADC01_CR2 & (1 << 3))
    {
    }
    // Chờ đến khi RSTCAL = 0
    //
    // Khi ADC hoàn thành reset calibration,
    // phần cứng tự động xóa bit RSTCAL


    ADC01_CR2 |= (1 << 2);
    // Set bit 2 = CAL
    //
    // Bắt đầu quá trình calibration ADC


    while (ADC01_CR2 & (1 << 2))
    {
    }
    // Chờ calibration hoàn thành
    //
    // Khi calibration xong,
    // phần cứng tự động xóa bit CAL


    ADC01_CR2 |= (1 << 0);
    // Set ADON = 1 lần nữa
    //
    // Đảm bảo ADC được bật sau quá trình calibration


    ADC01_CR2 |= (1 << 22);
    // Set bit 22 = SWSTART
    //
    // Bắt đầu quá trình chuyển đổi ADC bằng software
    //
    // Vì CONT = 1 nên sau lần chuyển đổi đầu tiên,
    // ADC sẽ tiếp tục chuyển đổi liên tục
}


uint16_t ADC01_CH3_Read(void)
{
    while (!(ADC01_SR & (1 << 1)))
    {
    }
    // Chờ bit EOC = 1
    //
    // EOC = End Of Conversion
    // Khi EOC = 1 nghĩa là ADC đã chuyển đổi xong
    // và dữ liệu đã có trong ADC_DR


    return (uint16_t)(ADC01_DR & 0x0FFF);
    // Đọc dữ liệu ADC từ ADC1_DR
    //
    // ADC của STM32F103 có độ phân giải 12 bit
    //
    // Giá trị:
    // 0     -> 0V
    // 4095  -> gần VREF
    //
    // 0x0FFF = 1111 1111 1111
    // Mask lấy 12 bit thấp
    //
    // Ép kiểu uint16_t để trả về giá trị 16 bit
}