#include "stm32f10x.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "mprintf.h"
#include "queue.h"



void LedConfig(){
	
	GPIO_InitTypeDef GPIO;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	GPIO.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
	GPIO.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO);
	
}

void LED1_TASK1(void *pvParamerters){
	while(1){
		GPIO_ResetBits(GPIOA,GPIO_Pin_0);
		vTaskDelay(5000);   // 0.1Hz -> nua chu ky 5000ms
		GPIO_SetBits(GPIOA,GPIO_Pin_0);
		vTaskDelay(5000);
	}
}

void LED2_TASK2(void *pvParamerters){
	while(1){
		GPIO_ResetBits(GPIOA,GPIO_Pin_1);
		vTaskDelay(500);    // 1Hz -> nua chu ky 500ms
		GPIO_SetBits(GPIOA,GPIO_Pin_1);
		vTaskDelay(500);
	}
}

void LED3_TASK3(void *pvParamerters){
	while(1){
		GPIO_ResetBits(GPIOA,GPIO_Pin_2);
		vTaskDelay(50);     // 10Hz -> nua chu ky 50ms
		GPIO_SetBits(GPIOA,GPIO_Pin_2);
		vTaskDelay(50);
	}
}

int main(){
	SystemInit();
	LedConfig();
	
	xTaskCreate(LED1_TASK1,"LED1",128,NULL,1,NULL);
	xTaskCreate(LED2_TASK2,"LED2",128,NULL,1,NULL);
	xTaskCreate(LED3_TASK3,"LED3",128,NULL,1,NULL);
	vTaskStartScheduler();
	while(1);
}