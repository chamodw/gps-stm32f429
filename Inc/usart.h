#ifndef __UART_H_
#define __UART_H_
#include "stm32f429i_discovery.h"
#include "stm32f4xx.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_usart.h"
#define USART1_BUFFER_SIZE 1024


void USART1_Init();
uint16_t USART1_NewData();
uint16_t USART1_CountChar(char c);
uint16_t USART1_GetData(uint8_t* dest, uint16_t size);
#endif